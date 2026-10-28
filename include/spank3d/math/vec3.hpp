#pragma once

namespace spank3d::math {

struct Vec3 {
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& rhs) const noexcept {
        return {x + rhs.x, y + rhs.y, z + rhs.z};
    }
    [[nodiscard]] constexpr Vec3 operator-(const Vec3& rhs) const noexcept {
        return {x - rhs.x, y - rhs.y, z - rhs.z};
    }
    [[nodiscard]] constexpr Vec3 operator*(float scalar) const noexcept {
        return {x * scalar, y * scalar, z * scalar};
    }
    [[nodiscard]] float length() const noexcept;
};

} // namespace spank3d::math
