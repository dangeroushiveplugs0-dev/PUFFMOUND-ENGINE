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
            var previousPinchDistance = 0f
            var previousMidX = 0f
            var previousMidY = 0f
            setOnTouchListener { _, event ->
                when (event.actionMasked) {
                    MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                        previousX = event.x
                        previousY = event.y
                        if (event.pointerCount >= 2) {
                            previousPinchDistance = pointerDistance(event)
                            previousMidX = (event.getX(0) + event.getX(1)) * 0.5f
                            previousMidY = (event.getY(0) + event.getY(1)) * 0.5f
                        }
                        true
                    }
                    MotionEvent.ACTION_MOVE -> {
                        if (event.pointerCount >= 2) {
                            val distance = pointerDistance(event)
                            val midX = (event.getX(0) + event.getX(1)) * 0.5f
                            val midY = (event.getY(0) + event.getY(1)) * 0.5f
                            if (previousPinchDistance > 0f && distance > 0f) {
                                val scale = distance / previousPinchDistance
                                queueEvent { nativeCameraZoom(scale) }
                            }
                            val panX = (midX - previousMidX) / height.coerceAtLeast(1)
                            val panY = (midY - previousMidY) / height.coerceAtLeast(1)
                            if (panX != 0f || panY != 0f) {
                                queueEvent { nativeCameraPan(panX * 2.2f, panY * 2.2f) }
                            }
                            previousPinchDistance = distance
                            previousMidX = midX
                            previousMidY = midY
                        } else {
                            val dx = (event.x - previousX) / width.coerceAtLeast(1)
                            val dy = (event.y - previousY) / height.coerceAtLeast(1)
                            queueEvent { nativeCameraOrbit(dx * 240f, dy * 240f) }
                        }
                        previousX = event.x
                        previousY = event.y
                        true
                    }
                    MotionEvent.ACTION_POINTER_UP -> {
                        previousPinchDistance = 0f
                        previousX = event.x
                        previousY = event.y
                        true
                    }
                    MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                        previousPinchDistance = 0f
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
