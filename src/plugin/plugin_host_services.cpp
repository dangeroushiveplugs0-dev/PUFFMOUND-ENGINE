#include "puffmound/plugin_host_services.hpp"

namespace puffmound {

PluginHostServices::PluginHostServices(PMLogFn log,
                                       void* log_user_data) noexcept
    : log_(log), log_user_data_(log_user_data) {
    host_.struct_size = sizeof(PMEngineHost);
    host_.api_major = PM_ENGINE_API_MAJOR;
    host_.api_minor = PM_ENGINE_API_MINOR;
    host_.user_data = this;
    host_.log = log_ == nullptr ? nullptr : &PluginHostServices::log_bridge;
    host_.register_frame_callback = &PluginHostServices::register_bridge;
    host_.unregister_frame_callback = &PluginHostServices::unregister_bridge;
}

const PMEngineHost* PluginHostServices::host() const noexcept {
    return &host_;
}

PMStatus PluginHostServices::dispatch_frame(const PMFrameContext& frame) {
    return callbacks_.dispatch(frame);
}

std::size_t PluginHostServices::callback_count() const noexcept {
    return callbacks_.size();
}

PMStatus PluginHostServices::register_bridge(void* host_user_data,
                                             PMFrameCallbackFn callback,
                                             void* callback_user_data,
                                             PMCallbackToken* out_token) {
    if (host_user_data == nullptr) return PM_STATUS_INVALID_ARGUMENT;
    auto* services = static_cast<PluginHostServices*>(host_user_data);
    return services->callbacks_.register_callback(callback, callback_user_data, out_token);
}

PMStatus PluginHostServices::unregister_bridge(void* host_user_data,
                                               PMCallbackToken token) {
    if (host_user_data == nullptr) return PM_STATUS_INVALID_ARGUMENT;
    auto* services = static_cast<PluginHostServices*>(host_user_data);
    return services->callbacks_.unregister_callback(token);
}

void PluginHostServices::log_bridge(void* host_user_data, PMLogLevel level,
                                    const char* message) {
    if (host_user_data == nullptr) return;
    auto* services = static_cast<PluginHostServices*>(host_user_data);
    if (services->log_ != nullptr) {
        services->log_(services->log_user_data_, level, message);
    }
}

} // namespace puffmound
