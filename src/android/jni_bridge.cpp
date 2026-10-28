#include "spank3d/render/gles_renderer.hpp"

#include <jni.h>

#include <cstdint>

namespace {
spank3d::render::GlesRenderer g_renderer;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_spank3d_MainActivity_nativeInitialize(JNIEnv*, jobject) {
    return g_renderer.initialize() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeResize(JNIEnv*, jobject, jint width, jint height) {
    const auto safe_width = width > 0 ? static_cast<std::uint32_t>(width) : 0U;
    const auto safe_height = height > 0 ? static_cast<std::uint32_t>(height) : 0U;
    g_renderer.resize({safe_width, safe_height});
}

extern "C" JNIEXPORT void JNICALL
Java_com_spank3d_MainActivity_nativeDraw(JNIEnv*, jobject) {
    g_renderer.begin_frame({0.035F, 0.045F, 0.075F, 1.0F});
    g_renderer.draw_debug_triangle();
    g_renderer.end_frame();
}
