#ifndef KIPEPEO_HOOK_MANAGER_H
#define KIPEPEO_HOOK_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Hook status enum
 */
typedef enum {
    HOOK_STATUS_UNINITIALIZED = 0,
    HOOK_STATUS_INITIALIZED = 1,
    HOOK_STATUS_ACTIVE_ROOT = 2,
    HOOK_STATUS_ACTIVE_NONROOT = 3,
    HOOK_STATUS_FAILED = 4
} HookStatus;

/**
 * Initialize the PLT hooking system
 * 
 * Attempts to hook system libraries for codec interception.
 * Falls back gracefully on non-root devices.
 * 
 * @return true if hooks were successfully installed, false otherwise
 */
bool hook_manager_init();

/**
 * Cleanup and remove all hooks
 */
void hook_manager_cleanup();

/**
 * Get current hook status
 * 
 * @return HookStatus enum value
 */
HookStatus hook_manager_get_status();

/**
 * Get human-readable status string
 * 
 * @return Status string (do not free)
 */
const char* hook_manager_get_status_string();

/**
 * Check if device has root privileges
 * 
 * @return true if running with root, false otherwise
 */
bool hook_manager_is_root_available();

#ifdef __cplusplus
}
#endif

#endif // KIPEPEO_HOOK_MANAGER_H
