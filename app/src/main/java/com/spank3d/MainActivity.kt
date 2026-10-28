package com.spank3d

import android.app.Activity
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.widget.FrameLayout
import android.widget.TextView
import android.graphics.Color
import android.view.Gravity
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import android.opengl.GLSurfaceView.Renderer

class MainActivity : Activity() {
    private lateinit var surface: GLSurfaceView
    private lateinit var status: TextView

    external fun nativeInitialize(): Boolean
    external fun nativeResize(width: Int, height: Int)
    external fun nativeDraw(elapsedSeconds: Float)
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

        setContentView(root)
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
