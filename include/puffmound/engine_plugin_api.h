#pragma once

/*
 * PUFFMOUND-ENGINE native extension ABI.
 *
 * This header defines the first public C-compatible contract only. It does not
 * implement package extraction, dynamic loading, scene access, or rendering.
 * Keep ABI-visible structures C-compatible and append fields only under the
 * documented struct_size/version rules.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PM_ENGINE_API_MAJOR 1u
#define PM_ENGINE_API_MINOR 0u
#define PM_PLUGIN_ABI_MAJOR 1u

typedef struct PMEngineHost PMEngineHost;
typedef struct PMFrameContext PMFrameContext;

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

/* Callbacks are invoked by the host; strings are UTF-8 and borrowed only
 * for the duration of the callback. Plugins must not retain these pointers. */
typedef void (*PMLogFn)(void* user_data, PMLogLevel level, const char* message);
typedef PMStatus (*PMPluginInitializeFn)(const PMEngineHost* host);
typedef void (*PMPluginShutdownFn)(void);

/*
 * Host service table. struct_size enables compatible future extension.
 * Plugins must check struct_size before reading fields added in later minors.
 * The host owns this table and guarantees it remains valid while the plugin
 * is active. Threading and callback registration rules will be specified by
 * the host implementation before callbacks are exposed.
 */
struct PMEngineHost {
    uint32_t struct_size;
    uint32_t api_major;
    uint32_t api_minor;
    void* user_data;
    PMLogFn log;
};

/*
 * Plugin descriptor exported by the plugin entry point. The host validates
 * struct_size, ABI major, and required engine API version before initialize.
 * The returned descriptor and strings must remain valid until shutdown.
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

/* Every native plugin exports this symbol with C linkage. The plugin must
 * return NULL if it cannot provide a valid descriptor for the requested ABI.
 */
typedef const PMPluginDescriptor* (*PMGetPluginDescriptorFn)(
    uint32_t requested_plugin_abi_major);

/* Compatible when the major version matches and the host minor is at least
 * the plugin's required minor version. Major versions are never interchangeable.
 */
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
