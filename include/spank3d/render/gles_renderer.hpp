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

    // Temporary 3D validation scene. Angle is in radians.
    void draw_debug_cube(float angle_radians);
    void shutdown() noexcept;

private:
    Viewport viewport_{};
    std::uint32_t cube_program_{0};
    std::uint32_t cube_vao_{0};
    std::uint32_t cube_vbo_{0};
    int mvp_location_{-1};
    bool initialized_{false};
};

} // namespace spank3d::render
