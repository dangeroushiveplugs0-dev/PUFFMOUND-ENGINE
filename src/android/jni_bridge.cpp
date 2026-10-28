#include "spank3d/render/gles_renderer.hpp"

#include <jni.h>

#include <cstdint>

namespace {
spank3d::render::GlesRenderer g_renderer;
float g_previous_elapsed_seconds = -1.0F;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_spank3d_MainActivity_nativeInitialize(JNIEnv*, jobject) {
    g_previous_elapsed_seconds = -1.0F;
    return g_renderer.initialize() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeResize(JNIEnv*, jobject, jint width, jint height) {
    const auto safe_width = width > 0 ? static_cast<std::uint32_t>(width) : 0U;
    const auto safe_height = height > 0 ? static_cast<std::uint32_t>(height) : 0U;
    g_renderer.resize({safe_width, safe_height});
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeDraw(JNIEnv*, jobject, jfloat elapsed_seconds) {
    if (g_previous_elapsed_seconds >= 0.0F && elapsed_seconds >= g_previous_elapsed_seconds) {
        g_renderer.camera_update(elapsed_seconds - g_previous_elapsed_seconds);
    }
    g_previous_elapsed_seconds = elapsed_seconds;
    g_renderer.begin_frame({0.035F, 0.045F, 0.075F, 1.0F});
    g_renderer.draw_debug_cube(0.0F);
    g_renderer.end_frame();
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeCameraOrbit(JNIEnv*, jobject, jfloat dx, jfloat dy) {
    g_renderer.camera_orbit(dx, dy);
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeCameraPan(JNIEnv*, jobject, jfloat dx, jfloat dy) {
    g_renderer.camera_pan(dx, dy);
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeCameraZoom(JNIEnv*, jobject, jfloat scale) {
    g_renderer.camera_zoom(scale);
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeCameraToggleProjection(JNIEnv*, jobject) {
    g_renderer.camera_toggle_projection();
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeCameraReset(JNIEnv*, jobject) {
    g_renderer.camera_reset();
}
