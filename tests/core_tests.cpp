#include "spank3d/math/vec3.hpp"
#include "puffmound/engine_plugin_api.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
bool near(float a, float b) { return std::fabs(a - b) < 1.0e-5F; }

int logged_messages = 0;
int initialize_calls = 0;
int shutdown_calls = 0;

void test_log(void*, PMLogLevel, const char* message) {
    if (message != nullptr) {
        ++logged_messages;
    }
}

PMStatus test_plugin_initialize(const PMEngineHost* host) {
    if (host == nullptr || host->struct_size < sizeof(PMEngineHost)) {
        return PM_STATUS_INVALID_ARGUMENT;
    }
    ++initialize_calls;
    if (host->log != nullptr) {
        host->log(host->user_data, PM_LOG_INFO, "test plugin initialized");
    }
    return PM_STATUS_OK;
}

void test_plugin_shutdown() { ++shutdown_calls; }

const PMPluginDescriptor* test_plugin_descriptor(uint32_t requested_abi) {
    static const PMPluginDescriptor descriptor = {
        sizeof(PMPluginDescriptor),
        PM_PLUGIN_ABI_MAJOR,
        "org.puffmound.tests.lifecycle",
        {1u, 0u, 0u},
        PM_ENGINE_API_MAJOR,
        PM_ENGINE_API_MINOR,
        test_plugin_initialize,
        test_plugin_shutdown
    };
    return requested_abi == PM_PLUGIN_ABI_MAJOR ? &descriptor : nullptr;
}

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}
}

int main() {
    static_assert(PM_ENGINE_API_MAJOR == 1u);
    static_assert(PM_PLUGIN_ABI_MAJOR == 1u);

    if (!check(pm_engine_api_is_compatible(1u, 2u, 1u, 1u),
               "Compatible minor-version negotiation failed")) return EXIT_FAILURE;
    if (!check(!pm_engine_api_is_compatible(1u, 0u, 1u, 1u),
               "Older host minor version was incorrectly accepted")) return EXIT_FAILURE;
    if (!check(!pm_engine_api_is_compatible(2u, 5u, 1u, 0u),
               "Mismatched major version was incorrectly accepted")) return EXIT_FAILURE;
    if (!check(!pm_engine_api_is_compatible(1u, 5u, 2u, 0u),
               "Plugin requiring a newer major version was accepted")) return EXIT_FAILURE;

    const PMPluginDescriptor* descriptor = test_plugin_descriptor(PM_PLUGIN_ABI_MAJOR);
    if (!check(descriptor != nullptr, "Plugin descriptor lookup failed")) return EXIT_FAILURE;
    if (!check(test_plugin_descriptor(PM_PLUGIN_ABI_MAJOR + 1u) == nullptr,
               "Plugin accepted an unsupported ABI request")) return EXIT_FAILURE;
    if (!check(descriptor->struct_size >= sizeof(PMPluginDescriptor),
               "Plugin descriptor struct_size is too small")) return EXIT_FAILURE;
    if (!check(descriptor->plugin_id != nullptr &&
                   std::strcmp(descriptor->plugin_id, "org.puffmound.tests.lifecycle") == 0,
               "Plugin descriptor ID was incorrect")) return EXIT_FAILURE;
    if (!check(descriptor->initialize != nullptr && descriptor->shutdown != nullptr,
               "Plugin lifecycle callbacks are missing")) return EXIT_FAILURE;
    if (!check(pm_engine_api_is_compatible(PM_ENGINE_API_MAJOR, PM_ENGINE_API_MINOR,
                                           descriptor->required_engine_api_major,
                                           descriptor->required_engine_api_minor),
               "Test plugin's required engine API was rejected")) return EXIT_FAILURE;

    PMEngineHost host = {
        sizeof(PMEngineHost),
        PM_ENGINE_API_MAJOR,
        PM_ENGINE_API_MINOR,
        nullptr,
        test_log
    };
    if (!check(descriptor->initialize(&host) == PM_STATUS_OK,
               "Plugin initialization callback failed")) return EXIT_FAILURE;
    if (!check(initialize_calls == 1 && logged_messages == 1,
               "Plugin initialization did not use the host logging service")) return EXIT_FAILURE;
    descriptor->shutdown();
    if (!check(shutdown_calls == 1, "Plugin shutdown callback was not invoked")) return EXIT_FAILURE;

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
