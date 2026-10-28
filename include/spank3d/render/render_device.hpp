#pragma once

#include <cstdint>

namespace spank3d::render {

struct ClearColor {
    float r{0.04F};
    float g{0.05F};
    float b{0.08F};
    float a{1.0F};
};

struct Viewport {
    std::uint32_t width{0};
    std::uint32_t height{0};
};

// Small backend-neutral contract; graphics API calls stay inside backend modules.
class RenderDevice {
public:
    virtual ~RenderDevice() = default;
    virtual bool initialize() = 0;
    virtual void resize(const Viewport& viewport) = 0;
    virtual void begin_frame(const ClearColor& color) = 0;
    virtual void end_frame() = 0;
    [[nodiscard]] virtual const char* backend_name() const noexcept = 0;
};

} // namespace spank3d::render
