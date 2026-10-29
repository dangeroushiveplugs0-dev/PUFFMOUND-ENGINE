#pragma once

#include "puffmound/engine_plugin_api.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace puffmound {

/*
 * Portable, single-update-thread callback registry used by the plugin host.
 * All methods must be called on the engine update thread. Callbacks may
 * unregister themselves or other callbacks while dispatch is in progress.
 */
class FrameCallbackRegistry {
public:
    PMStatus register_callback(PMFrameCallbackFn callback,
                               void* callback_user_data,
                               PMCallbackToken* out_token);
    PMStatus unregister_callback(PMCallbackToken token);
    PMStatus dispatch(const PMFrameContext& frame);
    std::size_t size() const noexcept;

private:
    struct Entry {
        PMCallbackToken token;
        PMFrameCallbackFn callback;
        void* user_data;
        bool active;
    };

    void compact_if_idle();

    std::vector<Entry> entries_;
    PMCallbackToken next_token_ = 1;
    std::size_t dispatch_depth_ = 0;
};

} // namespace puffmound
