#pragma once

#include "puffmound/engine_plugin_api.h"
#include "puffmound/frame_callback_registry.hpp"

namespace puffmound {

/*
 * Stable host-service adapter for native plugins. Keep this object alive and
 * at a fixed address for the entire lifetime of plugins using host().
 * All calls, including dispatch_frame(), belong on the engine update thread.
 */
class PluginHostServices {
public:
    explicit PluginHostServices(PMLogFn log = nullptr,
                                void* log_user_data = nullptr) noexcept;

    PluginHostServices(const PluginHostServices&) = delete;
    PluginHostServices& operator=(const PluginHostServices&) = delete;
    PluginHostServices(PluginHostServices&&) = delete;
    PluginHostServices& operator=(PluginHostServices&&) = delete;

    const PMEngineHost* host() const noexcept;
    PMStatus dispatch_frame(const PMFrameContext& frame);
    std::size_t callback_count() const noexcept;

private:
    static PMStatus register_bridge(void* host_user_data,
                                    PMFrameCallbackFn callback,
                                    void* callback_user_data,
                                    PMCallbackToken* out_token);
    static PMStatus unregister_bridge(void* host_user_data,
                                      PMCallbackToken token);
    static void log_bridge(void* host_user_data, PMLogLevel level,
                           const char* message);

    FrameCallbackRegistry callbacks_;
    PMEngineHost host_{};
    PMLogFn log_ = nullptr;
    void* log_user_data_ = nullptr;
};

} // namespace puffmound
