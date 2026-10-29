#include "puffmound/frame_callback_registry.hpp"

#include <algorithm>
#include <limits>

namespace puffmound {

PMStatus FrameCallbackRegistry::register_callback(PMFrameCallbackFn callback,
                                                  void* callback_user_data,
                                                  PMCallbackToken* out_token) {
    if (callback == nullptr || out_token == nullptr) {
        return PM_STATUS_INVALID_ARGUMENT;
    }
    if (next_token_ == 0 || next_token_ == std::numeric_limits<PMCallbackToken>::max()) {
        return PM_STATUS_INTERNAL_ERROR;
    }

    const PMCallbackToken token = next_token_++;
    entries_.push_back(Entry{token, callback, callback_user_data, true});
    *out_token = token;
    return PM_STATUS_OK;
}

PMStatus FrameCallbackRegistry::unregister_callback(PMCallbackToken token) {
    if (token == 0) {
        return PM_STATUS_INVALID_ARGUMENT;
    }

    const auto it = std::find_if(entries_.begin(), entries_.end(),
        [token](const Entry& entry) { return entry.token == token && entry.active; });
    if (it == entries_.end()) {
        return PM_STATUS_INVALID_ARGUMENT;
    }

    it->active = false;
    compact_if_idle();
    return PM_STATUS_OK;
}

PMStatus FrameCallbackRegistry::dispatch(const PMFrameContext& frame) {
    if (frame.struct_size < sizeof(PMFrameContext) ||
        frame.delta_seconds < 0.0 || frame.elapsed_seconds < 0.0) {
        return PM_STATUS_INVALID_ARGUMENT;
    }

    ++dispatch_depth_;
    PMStatus result = PM_STATUS_OK;

    // Snapshot tokens, not entry pointers: callbacks may mutate the registry.
    std::vector<PMCallbackToken> tokens;
    tokens.reserve(entries_.size());
    for (const Entry& entry : entries_) {
        if (entry.active) {
            tokens.push_back(entry.token);
        }
    }

    for (const PMCallbackToken token : tokens) {
        const auto it = std::find_if(entries_.begin(), entries_.end(),
            [token](const Entry& entry) { return entry.token == token && entry.active; });
        if (it == entries_.end()) {
            continue; // Removed by an earlier callback in this dispatch.
        }

        const PMStatus callback_status = it->callback(it->user_data, &frame);
        if (callback_status != PM_STATUS_OK && result == PM_STATUS_OK) {
            result = callback_status;
        }
    }

    --dispatch_depth_;
    compact_if_idle();
    return result;
}

std::size_t FrameCallbackRegistry::size() const noexcept {
    return static_cast<std::size_t>(std::count_if(entries_.begin(), entries_.end(),
        [](const Entry& entry) { return entry.active; }));
}

void FrameCallbackRegistry::compact_if_idle() {
    if (dispatch_depth_ != 0) {
        return;
    }
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
        [](const Entry& entry) { return !entry.active; }), entries_.end());
}

} // namespace puffmound
