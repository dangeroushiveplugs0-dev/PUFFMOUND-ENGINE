#include "spank3d/render/gles_renderer.hpp"

#include <GLES3/gl3.h>

namespace spank3d::render {

bool GlesRenderer::initialize() {
    // Android must provide a current EGL context before this is called.
    initialized_ = (glGetString(GL_VERSION) != nullptr);
    return initialized_;
}

void GlesRenderer::resize(const Viewport& viewport) {
    viewport_ = viewport;
    if (initialized_) {
        glViewport(0, 0, static_cast<GLsizei>(viewport.width),
                   static_cast<GLsizei>(viewport.height));
    }
}

void GlesRenderer::begin_frame(const ClearColor& color) {
    if (!initialized_) return;
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlesRenderer::end_frame() {
    // EGL buffer swapping is owned by the Android host/surface lifecycle.
}

const char* GlesRenderer::backend_name() const noexcept {
    return "OpenGL ES 3";
}

} // namespace spank3d::render
