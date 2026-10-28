#include "spank3d/math/vec3.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {
bool near(float a, float b) { return std::fabs(a - b) < 1.0e-5F; }
}

int main() {
    using spank3d::math::Vec3;
    const Vec3 a{1.0F, 2.0F, 3.0F};
    const Vec3 b{2.0F, -1.0F, 4.0F};
    const auto sum = a + b;
    const auto diff = a - b;
    if (!near(sum.x, 3.0F) || !near(sum.y, 1.0F) || !near(sum.z, 7.0F)) {
        std::cerr << "Vec3 addition failed\n"; return EXIT_FAILURE;
    }
    if (!near(diff.x, -1.0F) || !near(diff.y, 3.0F) || !near(diff.z, -1.0F)) {
        std::cerr << "Vec3 subtraction failed\n"; return EXIT_FAILURE;
    }
    if (!near(a.length(), std::sqrt(14.0F))) {
        std::cerr << "Vec3 length failed\n"; return EXIT_FAILURE;
    }
    std::cout << "Spank3d core tests passed\n";
    return EXIT_SUCCESS;
}
