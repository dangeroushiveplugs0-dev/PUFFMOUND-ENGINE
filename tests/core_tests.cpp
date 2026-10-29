#include "spank3d/math/vec3.hpp"
#include "puffmound/engine_plugin_api.h"
#include "puffmound/frame_callback_registry.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
bool near(float a, float b) { return std::fabs(a - b) < 1.0e-5F; }

int logged_messages = 0;
int initialize_calls = 0;
int shutdown_calls = 0;
int frame_calls = 0;
PMFrameCallbackFn registered_callback = nullptr;
void* registered_callback_user_data = nullptr;
PMCallbackToken registered_token = 0;
PMCallbackToken next_token = 1;

void test_log(void*, PMLogLevel, const char* message) {
    if (message != nullptr) ++logged_messages;
}

PMStatus test_plugin_initialize(const PMEngineHost* host) {
    if (host == nullptr || host->struct_size < sizeof(PMEngineHost)) {
        return PM_STATUS_INVALID_ARGUMENT;
    }
    ++initialize_calls;
    if (host->log != nullptr) host->log(host->user_data, PM_LOG_INFO, "test plugin initialized");
    return PM_STATUS_OK;
}

void test_plugin_shutdown() { ++shutdown_calls; }

PMStatus register_mock_callback(void*, PMFrameCallbackFn callback,
                                void* callback_user_data, PMCallbackToken* out_token) {
    if (callback == nullptr || out_token == nullptr || registered_callback != nullptr) {
        return PM_STATUS_INVALID_ARGUMENT;
    }
    registered_callback = callback;
    registered_callback_user_data = callback_user_data;
    registered_token = next_token++;
    *out_token = registered_token;
    return PM_STATUS_OK;
}

PMStatus unregister_mock_callback(void*, PMCallbackToken token) {
    if (registered_callback == nullptr || token == 0 || token != registered_token) {
        return PM_STATUS_INVALID_ARGUMENT;
    }
    registered_callback = nullptr;
    registered_callback_user_data = nullptr;
    registered_token = 0;
    return PM_STATUS_OK;
}

PMStatus test_frame_callback(void* user_data, const PMFrameContext* frame) {
    if (user_data == nullptr || frame == nullptr ||
        frame->struct_size < sizeof(PMFrameContext) ||
        frame->delta_seconds <= 0.0 || frame->elapsed_seconds < frame->delta_seconds) {
        return PM_STATUS_INVALID_ARGUMENT;
    }
    ++frame_calls;
    return PM_STATUS_OK;
}

struct RegistryCallbackState {
    puffmound::FrameCallbackRegistry* registry;
    PMCallbackToken token = 0;
    int calls = 0;
    bool unregister_self = false;
    PMStatus return_status = PM_STATUS_OK;
};

PMStatus registry_callback(void* raw_state, const PMFrameContext*) {
    auto* state = static_cast<RegistryCallbackState*>(raw_state);
    if (state == nullptr) return PM_STATUS_INVALID_ARGUMENT;
    ++state->calls;
    if (state->unregister_self) {
        const PMStatus status = state->registry->unregister_callback(state->token);
        if (status != PM_STATUS_OK) return status;
    }
    return state->return_status;
}

const PMPluginDescriptor* test_plugin_descriptor(uint32_t requested_abi) {
    static const PMPluginDescriptor descriptor = {
        sizeof(PMPluginDescriptor), PM_PLUGIN_ABI_MAJOR,
        "org.puffmound.tests.lifecycle", {1u, 0u, 0u},
        PM_ENGINE_API_MAJOR, PM_ENGINE_API_MINOR,
        test_plugin_initialize, test_plugin_shutdown
    };
    return requested_abi == PM_PLUGIN_ABI_MAJOR ? &descriptor : nullptr;
}

bool check(bool condition, const char* message) {
    if (!condition) std::cerr << message << '\n';
    return condition;
}

bool test_real_registry() {
    puffmound::FrameCallbackRegistry registry;
    PMCallbackToken invalid_token = 0;
    if (!check(registry.register_callback(nullptr, nullptr, &invalid_token) ==
                   PM_STATUS_INVALID_ARGUMENT,
               "Registry accepted a null callback")) return false;
    if (!check(registry.register_callback(registry_callback, nullptr, nullptr) ==
                   PM_STATUS_INVALID_ARGUMENT,
               "Registry accepted a null output token")) return false;

    RegistryCallbackState first{&registry};
    RegistryCallbackState self_removing{&registry};
    self_removing.unregister_self = true;
    RegistryCallbackState failing{&registry};
    failing.return_status = PM_STATUS_INTERNAL_ERROR;

    PMCallbackToken first_token = 0;
    PMCallbackToken self_token = 0;
    PMCallbackToken failing_token = 0;
    if (!check(registry.register_callback(registry_callback, &first, &first_token) == PM_STATUS_OK,
               "Registry failed to add first callback")) return false;
    self_removing.token = 0;
    if (!check(registry.register_callback(registry_callback, &self_removing, &self_token) == PM_STATUS_OK,
               "Registry failed to add self-removing callback")) return false;
    self_removing.token = self_token;
    if (!check(registry.register_callback(registry_callback, &failing, &failing_token) == PM_STATUS_OK,
               "Registry failed to add failing callback")) return false;
    if (!check(registry.size() == 3, "Registry size did not reflect registrations")) return false;

    const PMFrameContext frame{sizeof(PMFrameContext), 3u, 1.0 / 60.0, 0.05};
    if (!check(registry.dispatch(frame) == PM_STATUS_INTERNAL_ERROR,
               "Registry did not propagate callback failure")) return false;
    if (!check(first.calls == 1 && self_removing.calls == 1 && failing.calls == 1,
               "Registry did not dispatch all callbacks exactly once")) return false;
    if (!check(registry.size() == 2, "Self-unregistered callback was not removed")) return false;

    if (!check(registry.unregister_callback(self_token) == PM_STATUS_INVALID_ARGUMENT,
               "Registry accepted duplicate unregistration")) return false;
    if (!check(registry.unregister_callback(first_token) == PM_STATUS_OK,
               "Registry failed to remove callback by token")) return false;
    if (!check(registry.unregister_callback(failing_token) == PM_STATUS_OK,
               "Registry failed to remove failing callback")) return false;
    if (!check(registry.size() == 0, "Registry retained callbacks after removal")) return false;
    if (!check(registry.dispatch(frame) == PM_STATUS_OK,
               "Empty registry dispatch should succeed")) return false;

    const PMFrameContext invalid_frame{sizeof(PMFrameContext), 4u, -0.1, 0.1};
    return check(registry.dispatch(invalid_frame) == PM_STATUS_INVALID_ARGUMENT,
                 "Registry accepted invalid frame timing");
}
}

int main() {
    static_assert(PM_ENGINE_API_MAJOR == 1u);
    static_assert(PM_ENGINE_API_MINOR == 1u);
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
        sizeof(PMEngineHost), PM_ENGINE_API_MAJOR, PM_ENGINE_API_MINOR,
        nullptr, test_log, register_mock_callback, unregister_mock_callback
    };
    if (!check(descriptor->initialize(&host) == PM_STATUS_OK,
               "Plugin initialization callback failed")) return EXIT_FAILURE;
    if (!check(initialize_calls == 1 && logged_messages == 1,
               "Plugin initialization did not use host logging")) return EXIT_FAILURE;

    int plugin_state = 42;
    PMCallbackToken callback_token = 0;
    if (!check(host.register_frame_callback(host.user_data, test_frame_callback,
                                            &plugin_state, &callback_token) == PM_STATUS_OK,
               "Mock host rejected valid frame callback")) return EXIT_FAILURE;
    if (!check(callback_token != 0 && registered_callback == test_frame_callback &&
                   registered_callback_user_data == &plugin_state,
               "Mock host did not retain callback")) return EXIT_FAILURE;
    const PMFrameContext frame = {sizeof(PMFrameContext), 17u, 1.0 / 60.0, 0.5};
    if (!check(registered_callback(registered_callback_user_data, &frame) == PM_STATUS_OK &&
                   frame_calls == 1,
               "Mock callback did not receive valid frame timing")) return EXIT_FAILURE;
    if (!check(host.unregister_frame_callback(host.user_data, callback_token) == PM_STATUS_OK,
               "Mock host failed to unregister callback")) return EXIT_FAILURE;
    if (!check(registered_callback == nullptr,
               "Mock host retained unregistered callback")) return EXIT_FAILURE;
    if (!check(host.unregister_frame_callback(host.user_data, callback_token) ==
                   PM_STATUS_INVALID_ARGUMENT,
               "Mock host accepted duplicate unregistration")) return EXIT_FAILURE;
    descriptor->shutdown();
    if (!check(shutdown_calls == 1, "Plugin shutdown callback was not invoked")) return EXIT_FAILURE;

    if (!test_real_registry()) return EXIT_FAILURE;

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
