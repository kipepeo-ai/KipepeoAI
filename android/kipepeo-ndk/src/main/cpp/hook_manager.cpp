#include "hook_manager.h"
#include "mediacodec_hooks.h"
#include <xhook.h>
#include <android/log.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <dlfcn.h>

#define TAG "KipepeoHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

// Global hook status
static HookStatus g_hook_status = HOOK_STATUS_UNINITIALIZED;
static bool g_has_root = false;

/**
 * Check if running with root privileges
 */
static bool check_root_access() {
    // Method 1: Check if /system is writable
    if (access("/system/xbin/su", X_OK) == 0 || 
        access("/system/bin/su", X_OK) == 0 ||
        access("/sbin/su", X_OK) == 0) {
        LOGI("Root binary detected");
        return true;
    }
    
    // Method 2: Check effective UID
    if (geteuid() == 0) {
        LOGI("Running as root (UID 0)");
        return true;
    }
    
    LOGI("No root access detected");
    return false;
}

/**
 * Initialize hooks for system libraries
 */
static bool init_system_hooks() {
    LOGI("Attempting to hook system libraries...");
    
    int result = 0;
    
    // Hook libstagefright.so (MediaCodec implementation)
    result |= xhook_register(
        ".*libstagefright\\.so$",
        "AMediaCodec_createEncoderByType",
        (void*)hooked_create_encoder,
        NULL
    );
    
    result |= xhook_register(
        ".*libstagefright\\.so$",
        "AMediaCodec_createDecoderByType",
        (void*)hooked_create_decoder,
        NULL
    );
    
    // Hook libmedia.so
    result |= xhook_register(
        ".*libmedia\\.so$",
        "AMediaCodec_createEncoderByType",
        (void*)hooked_create_encoder,
        NULL
    );
    
    result |= xhook_register(
        ".*libmedia\\.so$",
        "AMediaCodec_createDecoderByType",
        (void*)hooked_create_decoder,
        NULL
    );
    
    // Hook libavcodec.so (for apps that bundle FFmpeg)
    result |= xhook_register(
        ".*libavcodec\\.so.*$",
        "avcodec_find_encoder",
        (void*)hooked_avcodec_find_encoder,
        NULL
    );
    
    result |= xhook_register(
        ".*libavcodec\\.so.*$",
        "avcodec_find_decoder",
        (void*)hooked_avcodec_find_decoder,
        NULL
    );
    
    // Refresh to apply hooks
    if (xhook_refresh(0) == 0) {
        LOGI("System hooks successfully installed");
        return true;
    } else {
        LOGE("Failed to refresh xhook (code: %d)", result);
        return false;
    }
}

bool hook_manager_init() {
    LOGI("Initializing Kipepeo Hook Manager...");
    
    if (g_hook_status != HOOK_STATUS_UNINITIALIZED) {
        LOGD("Hook manager already initialized (status: %d)", g_hook_status);
        return g_hook_status == HOOK_STATUS_ACTIVE_ROOT || 
               g_hook_status == HOOK_STATUS_ACTIVE_NONROOT;
    }
    
    // Initialize xhook
    xhook_enable_debug(1);  // Enable debug logging
    xhook_enable_sigsegv_protection(1);  // Protect against crashes
    
    // Check root status
    g_has_root = check_root_access();
    
    // Initialize MediaCodec hooks
    if (!mediacodec_hooks_init()) {
        LOGE("Failed to initialize MediaCodec hooks");
        g_hook_status = HOOK_STATUS_FAILED;
        return false;
    }
    
    // Try to install system hooks
    bool hooks_installed = init_system_hooks();
    
    if (hooks_installed) {
        g_hook_status = g_has_root ? HOOK_STATUS_ACTIVE_ROOT : HOOK_STATUS_ACTIVE_NONROOT;
        LOGI("Hook manager initialized successfully (mode: %s)", 
             g_has_root ? "ROOT" : "NON-ROOT");
        return true;
    } else {
        if (g_has_root) {
            // Even with root, hooks failed - this is an error
            LOGE("Failed to install hooks despite having root access");
            g_hook_status = HOOK_STATUS_FAILED;
            return false;
        } else {
            // Non-root - fallback mode (VPN-based tracking only)
            LOGI("Running in non-root fallback mode (limited functionality)");
            g_hook_status = HOOK_STATUS_ACTIVE_NONROOT;
            return true;  // Still consider it "successful" in non-root mode
        }
    }
}

void hook_manager_cleanup() {
    LOGI("Cleaning up Kipepeo Hook Manager...");
    
    if (g_hook_status == HOOK_STATUS_UNINITIALIZED) {
        LOGD("Hook manager not initialized, nothing to clean up");
        return;
    }
    
    // Cleanup MediaCodec hooks
    mediacodec_hooks_cleanup();
    
    // Clear xhook
    xhook_clear();
    
    g_hook_status = HOOK_STATUS_UNINITIALIZED;
    LOGI("Hook manager cleaned up");
}

HookStatus hook_manager_get_status() {
    return g_hook_status;
}

const char* hook_manager_get_status_string() {
    switch (g_hook_status) {
        case HOOK_STATUS_UNINITIALIZED:
            return "Uninitialized";
        case HOOK_STATUS_INITIALIZED:
            return "Initialized";
        case HOOK_STATUS_ACTIVE_ROOT:
            return "Active (Root Mode)";
        case HOOK_STATUS_ACTIVE_NONROOT:
            return "Active (Non-Root Mode - Limited)";
        case HOOK_STATUS_FAILED:
            return "Failed";
        default:
            return "Unknown";
    }
}

bool hook_manager_is_root_available() {
    return g_has_root;
}
