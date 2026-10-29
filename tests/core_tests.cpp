#include "spank3d/math/vec3.hpp"
#include "puffmound/engine_plugin_api.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {
bool near(float a, float b) { return std::fabs(a - b) < 1.0e-5F; }
}

int main() {
    static_assert(PM_ENGINE_API_MAJOR == 1u);
    static_assert(PM_PLUGIN_ABI_MAJOR == 1u);

    if (!pm_engine_api_is_compatible(1u, 2u, 1u, 1u)) {
        std::cerr << "Compatible minor-version negotiation failed\n";
        return EXIT_FAILURE;
    }
    if (pm_engine_api_is_compatible(1u, 0u, 1u, 1u)) {
        std::cerr << "Older host minor version was incorrectly accepted\n";
        return EXIT_FAILURE;
    }
    if (pm_engine_api_is_compatible(2u, 5u, 1u, 0u)) {
        std::cerr << "Mismatched major version was incorrectly accepted\n";
        return EXIT_FAILURE;
    }
    if (pm_engine_api_is_compatible(1u, 5u, 2u, 0u)) {
        std::cerr << "Plugin requiring a newer major version was accepted\n";
        return EXIT_FAILURE;
    }

    using spank3d::math::Vec3;
    const Vec3 a{1.0F, 2.0F, 3.0F};
    const Vec3 b{2.0F, -1.0F, 4.0F};
    const auto sum = a + b;
    const auto diff = a - b;
    if (!near(sum.x, 3.0F) || !near(sum.y, 1.0F) || !near(sum.z, 7.0F)) {
        std::cerr << "Vec3 addition failed\n";
        return EXIT_FAILURE;
    }
    if (!near(diff.x, -1.0F) || !near(diff.y, 3.0F) || !near(diff.z, -1.0F)) {
        std::cerr << "Vec3 subtraction failed\n";
        return EXIT_FAILURE;
    }
    if (!near(a.length(), std::sqrt(14.0F))) {
        std::cerr << "Vec3 length failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "PUFFMOUND core and plugin ABI tests passed\n";
    return EXIT_SUCCESS;
}
