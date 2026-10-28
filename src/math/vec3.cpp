#include "spank3d/math/vec3.hpp"

#include <cmath>

namespace spank3d::math {

float Vec3::length() const noexcept {
    return std::sqrt(x * x + y * y + z * z);
}

} // namespace spank3d::math
