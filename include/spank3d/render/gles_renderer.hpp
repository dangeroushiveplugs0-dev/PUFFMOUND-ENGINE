#pragma once

#include "spank3d/render/render_device.hpp"

namespace spank3d::render {

class GlesRenderer final : public RenderDevice {
public:
    bool initialize() override;
    void resize(const Viewport& viewport) override;
    void begin_frame(const ClearColor& color) override;
    void end_frame() override;
    [[nodiscard]] const char* backend_name() const noexcept override;

private:
    Viewport viewport_{};
    bool initialized_{false};
};

} // namespace spank3d::render
