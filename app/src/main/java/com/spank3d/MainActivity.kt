package com.spank3d

import android.app.Activity
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.widget.FrameLayout
import android.widget.TextView
import android.graphics.Color
import android.view.Gravity
import android.view.MotionEvent
import android.widget.Button
import android.widget.LinearLayout
import kotlin.math.hypot
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import android.opengl.GLSurfaceView.Renderer

class MainActivity : Activity() {
    private lateinit var surface: GLSurfaceView
    private lateinit var status: TextView

    external fun nativeInitialize(): Boolean
    external fun nativeResize(width: Int, height: Int)
    external fun nativeDraw(elapsedSeconds: Float)
    external fun nativeCameraOrbit(dx: Float, dy: Float)
    external fun nativeCameraPan(dx: Float, dy: Float)
    external fun nativeCameraZoom(scale: Float)
    external fun nativeCameraToggleProjection()
    external fun nativeCameraReset()
    private var startTimeNanos: Long = 0L

    companion object {
        init {
            System.loadLibrary("spank3d_android")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val root = FrameLayout(this)
        surface = GLSurfaceView(this).apply {
            setEGLContextClientVersion(3)
            preserveEGLContextOnPause = false
            setRenderer(object : Renderer {
                override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
                    startTimeNanos = System.nanoTime()
                    val initialized = nativeInitialize()
                    runOnUiThread {
                        status.text = if (initialized) {
                            "Spank3d • OpenGL ES 3 • 3D cube"
                        } else {
                            "Spank3d • Renderer initialization failed"
                        }
                    }
                }

                override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
                    nativeResize(width, height)
                }

                override fun onDrawFrame(gl: GL10?) {
                    val elapsedSeconds = ((System.nanoTime() - startTimeNanos).coerceAtLeast(0L)) / 1_000_000_000.0
                    nativeDraw(elapsedSeconds.toFloat())
                }
            })
            renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
            var previousX = 0f
            var previousY = 0f
            var previousPointerCount = 0
            var previousSpread = 0f
            var previousCentroidX = 0f
            var previousCentroidY = 0f

            fun rebaseGesture(event: MotionEvent, excludedPointer: Int = -1) {
                val count = event.pointerCount - if (excludedPointer >= 0) 1 else 0
                if (count <= 0) {
                    previousPointerCount = 0
                    previousSpread = 0f
                    return
                }
                var sumX = 0f
                var sumY = 0f
                for (i in 0 until event.pointerCount) {
                    if (i == excludedPointer) continue
                    sumX += event.getX(i)
                    sumY += event.getY(i)
                }
                previousCentroidX = sumX / count
                previousCentroidY = sumY / count
                var squaredDistanceSum = 0f
                for (i in 0 until event.pointerCount) {
                    if (i == excludedPointer) continue
                    val dx = event.getX(i) - previousCentroidX
                    val dy = event.getY(i) - previousCentroidY
                    squaredDistanceSum += dx * dx + dy * dy
                }
                previousSpread = kotlin.math.sqrt(squaredDistanceSum / count)
                if (count == 1) {
                    for (i in 0 until event.pointerCount) {
                        if (i != excludedPointer) {
                            previousX = event.getX(i)
                            previousY = event.getY(i)
                            break
                        }
                    }
                }
                previousPointerCount = count
            }

            setOnTouchListener { _, event ->
                when (event.actionMasked) {
                    MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                        // A finger-count change starts a fresh gesture baseline, preventing jumps.
                        rebaseGesture(event)
                        true
                    }
                    MotionEvent.ACTION_MOVE -> {
                        val count = event.pointerCount
                        if (count != previousPointerCount) {
                            rebaseGesture(event)
                            true
                        } else if (count == 1) {
                            val x = event.getX(0)
                            val y = event.getY(0)
                            val dx = (x - previousX) / width.coerceAtLeast(1)
                            val dy = (y - previousY) / height.coerceAtLeast(1)
                            if (dx != 0f || dy != 0f) {
                                queueEvent { nativeCameraOrbit(dx * 240f, dy * 240f) }
                            }
                            previousX = x
                            previousY = y
                            true
                        } else {
                            // Two or more fingers: centroid pans while spread pinch-zooms.
                            var sumX = 0f
                            var sumY = 0f
                            for (i in 0 until count) {
                                sumX += event.getX(i)
                                sumY += event.getY(i)
                            }
                            val centroidX = sumX / count
                            val centroidY = sumY / count
                            var squaredDistanceSum = 0f
                            for (i in 0 until count) {
                                val dx = event.getX(i) - centroidX
                                val dy = event.getY(i) - centroidY
                                squaredDistanceSum += dx * dx + dy * dy
                            }
                            val spread = kotlin.math.sqrt(squaredDistanceSum / count)
                            val moveX = (centroidX - previousCentroidX) / width.coerceAtLeast(1)
                            val moveY = (centroidY - previousCentroidY) / height.coerceAtLeast(1)
                            val scale = if (previousSpread > 1f && spread > 1f) {
                                spread / previousSpread
                            } else {
                                1f
                            }
                            if (moveX != 0f || moveY != 0f || scale != 1f) {
                                queueEvent {
                                    // Two fingers orbit vertically/horizontally and pinch-zoom.
                                    // Three or more fingers pan and pinch-zoom.
                                    if (count == 2) {
                                        if (moveX != 0f || moveY != 0f) {
                                            nativeCameraOrbit(moveX * 240f, moveY * 240f)
                                        }
                                    } else if (moveX != 0f || moveY != 0f) {
                                        nativeCameraPan(moveX * 2.2f, moveY * 2.2f)
                                    }
                                    if (scale != 1f) nativeCameraZoom(scale)
                                }
                            }
                            previousCentroidX = centroidX
                            previousCentroidY = centroidY
                            previousSpread = spread
                            true
                        }
                    }
                    MotionEvent.ACTION_POINTER_UP -> {
                        // Exclude the lifted pointer so the remaining fingers continue smoothly.
                        rebaseGesture(event, event.actionIndex)
                        true
                    }
                    MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                        previousPointerCount = 0
                        previousSpread = 0f
                        true
                    }
                    else -> true
                }
            }
        }
        root.addView(
            surface,
            FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
        )

        status = TextView(this).apply {
            text = "Spank3d • Starting renderer…"
            setTextColor(Color.WHITE)
            textSize = 12f
            setPadding(18, 12, 18, 12)
            setBackgroundColor(0xAA101522.toInt())
            gravity = Gravity.CENTER_VERTICAL
            contentDescription = "Renderer status"
        }
        val statusParams = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT,
            Gravity.TOP or Gravity.CENTER_HORIZONTAL
        )
        statusParams.topMargin = 16
        root.addView(status, statusParams)

        val controls = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER
        }
        val projectionButton = Button(this).apply {
            text = "Perspective / Ortho"
            isAllCaps = false
            setOnClickListener { surface.queueEvent { nativeCameraToggleProjection() } }
        }
        val resetButton = Button(this).apply {
            text = "Reset camera"
            isAllCaps = false
            setOnClickListener { surface.queueEvent { nativeCameraReset() } }
        }
        controls.addView(projectionButton)
        controls.addView(resetButton)
        val controlsParams = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT,
            Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
        )
        controlsParams.bottomMargin = 18
        root.addView(controls, controlsParams)

        setContentView(root)
    }

    private fun pointerDistance(event: MotionEvent): Float {
        if (event.pointerCount < 2) return 0f
        val dx = event.getX(0) - event.getX(1)
        val dy = event.getY(0) - event.getY(1)
        return hypot(dx, dy)
    }

    override fun onPause() {
        surface.onPause()
        super.onPause()
    }

    override fun onResume() {
        super.onResume()
        surface.onResume()
    }
}
