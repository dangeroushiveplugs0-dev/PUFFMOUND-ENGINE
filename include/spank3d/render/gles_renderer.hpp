#pragma once

#include "spank3d/render/render_device.hpp"

#include <cstdint>

namespace spank3d::render {

// OpenGL ES 3 backend. The host must keep an EGL context current for every
// method that issues GL calls, including shutdown().
class GlesRenderer final : public RenderDevice {
public:
    bool initialize() override;
    void resize(const Viewport& viewport) override;
    void begin_frame(const ClearColor& color) override;
    void end_frame() override;
    [[nodiscard]] const char* backend_name() const noexcept override;

    // Small end-to-end draw path used to validate shader compilation and buffers.
    void draw_debug_triangle();
    void shutdown() noexcept;

private:
    Viewport viewport_{};
    std::uint32_t triangle_program_{0};
    std::uint32_t triangle_vao_{0};
    std::uint32_t triangle_vbo_{0};
    bool initialized_{false};
};

} // namespace spank3d::render
