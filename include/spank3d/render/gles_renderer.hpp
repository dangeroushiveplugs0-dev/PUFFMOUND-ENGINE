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
    void camera_orbit(float delta_x, float delta_y) noexcept;
    void camera_pan(float delta_x, float delta_y) noexcept;
    void camera_zoom(float scale_factor) noexcept;
    void camera_toggle_projection() noexcept;
    void camera_reset() noexcept;
    [[nodiscard]] bool camera_is_orthographic() const noexcept { return orthographic_; }
    void shutdown() noexcept;

private:
    Viewport viewport_{};
    std::uint32_t cube_program_{0};
    std::uint32_t cube_vao_{0};
    std::uint32_t cube_vbo_{0};
    int mvp_location_{-1};
    bool initialized_{false};
    float camera_yaw_{0.55F};
    float camera_pitch_{0.32F};
    float camera_distance_{4.8F};
    float camera_target_x_{0.0F};
    float camera_target_y_{0.0F};
    bool orthographic_{false};
};

} // namespace spank3d::render
