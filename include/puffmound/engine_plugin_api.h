#pragma once

/*
 * PUFFMOUND-ENGINE native extension ABI.
 *
 * This header defines a C-compatible contract only. It does not implement
 * package extraction, dynamic loading, scene access, or rendering.
 * ABI-visible structures use struct_size for forward-compatible extension.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PM_ENGINE_API_MAJOR 1u
#define PM_ENGINE_API_MINOR 1u
#define PM_PLUGIN_ABI_MAJOR 1u

typedef struct PMEngineHost PMEngineHost;
typedef struct PMFrameContext PMFrameContext;
typedef uint64_t PMCallbackToken;

typedef enum PMStatus {
    PM_STATUS_OK = 0,
    PM_STATUS_INVALID_ARGUMENT = 1,
    PM_STATUS_UNSUPPORTED_VERSION = 2,
    PM_STATUS_INITIALIZATION_FAILED = 3,
    PM_STATUS_INTERNAL_ERROR = 4
} PMStatus;

typedef enum PMLogLevel {
    PM_LOG_DEBUG = 0,
    PM_LOG_INFO = 1,
    PM_LOG_WARNING = 2,
    PM_LOG_ERROR = 3
} PMLogLevel;

typedef struct PMVersion {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} PMVersion;

/* Frame timing is supplied by the host and is read-only to plugins. */
struct PMFrameContext {
    uint32_t struct_size;
    uint64_t frame_index;
    double delta_seconds;
    double elapsed_seconds;
};

/* The host invokes a registered callback during its frame-update phase.
 * callback_user_data is owned by the plugin; the host does not free it.
 * A plugin must unregister callbacks before its shutdown callback returns.
 */
typedef PMStatus (*PMFrameCallbackFn)(
    void* callback_user_data,
    const PMFrameContext* frame);

typedef void (*PMLogFn)(void* user_data, PMLogLevel level, const char* message);
typedef PMStatus (*PMPluginInitializeFn)(const PMEngineHost* host);
typedef void (*PMPluginShutdownFn)(void);

typedef PMStatus (*PMRegisterFrameCallbackFn)(
    void* host_user_data,
    PMFrameCallbackFn callback,
    void* callback_user_data,
    PMCallbackToken* out_token);

typedef PMStatus (*PMUnregisterFrameCallbackFn)(
    void* host_user_data,
    PMCallbackToken token);

/*
 * Host service table. The host owns this table and keeps it valid while the
 * plugin is active. Plugins must check struct_size before reading fields.
 * Callback registration is host-controlled; frame callbacks run on the
 * engine's update thread, not an arbitrary worker thread. The host must stop
 * invoking a callback before unregister returns successfully.
 */
struct PMEngineHost {
    uint32_t struct_size;
    uint32_t api_major;
    uint32_t api_minor;
    void* user_data;
    PMLogFn log;
    PMRegisterFrameCallbackFn register_frame_callback;
    PMUnregisterFrameCallbackFn unregister_frame_callback;
};

/*
 * Plugin descriptor exported by the plugin entry point. The host validates
 * struct_size, ABI major, and required engine API version before initialize.
 * The descriptor and strings must remain valid until shutdown completes.
 */
typedef struct PMPluginDescriptor {
    uint32_t struct_size;
    uint32_t plugin_abi_major;
    const char* plugin_id;
    PMVersion plugin_version;
    uint32_t required_engine_api_major;
    uint32_t required_engine_api_minor;
    PMPluginInitializeFn initialize;
    PMPluginShutdownFn shutdown;
} PMPluginDescriptor;

/* Every native plugin exports this symbol with C linkage. */
typedef const PMPluginDescriptor* (*PMGetPluginDescriptorFn)(
    uint32_t requested_plugin_abi_major);

/* Major versions must match; host minor must meet the plugin's requirement. */
static inline int pm_engine_api_is_compatible(
    uint32_t host_major,
    uint32_t host_minor,
    uint32_t required_major,
    uint32_t required_minor) {
    return host_major == required_major && host_minor >= required_minor;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
