#include "native_interface.h"
#include "hook_manager.h"
#include "mediacodec_hooks.h"
#include "data_tracker.h"
#include <android/log.h>
#include <string.h>

#define TAG "KipepeoNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static bool g_initialized = false;
static bool g_engine_active = false;

bool kipepeo_init() {
    if (g_initialized) {
        LOGI("Kipepeo already initialized");
        return true;
    }
    
    LOGI("Initializing Kipepeo Native Interface...");
    
    // Initialize data tracker
    if (!data_tracker_init()) {
        LOGE("Failed to initialize data tracker");
        return false;
    }
    
    g_initialized = true;
    LOGI("Kipepeo initialization complete");
    return true;
}

void kipepeo_shutdown() {
    if (!g_initialized) {
        return;
    }
    
    LOGI("Shutting down Kipepeo...");
    
    // Deactivate engine if active
    if (g_engine_active) {
        kipepeo_deactivate_engine();
    }
    
    // Cleanup data tracker
    data_tracker_cleanup();
    
    g_initialized = false;
    LOGI("Kipepeo shutdown complete");
}

bool kipepeo_activate_engine() {
    if (!g_initialized) {
        LOGE("Cannot activate engine: Kipepeo not initialized");
        return false;
    }
    
    if (g_engine_active) {
        LOGI("Engine already active");
        return true;
    }
    
    LOGI("Activating Kipepeo Engine...");
    
    // Initialize hook manager
    if (!hook_manager_init()) {
        LOGE("Failed to initialize hook manager");
        return false;
    }
    
    g_engine_active = true;
    LOGI("Kipepeo Engine activated");
    return true;
}

void kipepeo_deactivate_engine() {
    if (!g_engine_active) {
        return;
    }
    
    LOGI("Deactivating Kipepeo Engine...");
    
    // Cleanup hook manager
    hook_manager_cleanup();
    
    g_engine_active = false;
    LOGI("Kipepeo Engine deactivated");
}

bool kipepeo_is_root_available() {
    return hook_manager_is_root_available();
}

const char* kipepeo_get_hook_status() {
    if (!g_engine_active) {
        return "Inactive";
    }
    
    return hook_manager_get_status_string();
}

float kipepeo_get_tokens_per_second() {
    // TODO: Implement LLM inference metric tracking
    // For now, return 0.0 to indicate no LLM inference running
    return 0.0f;
}

uint64_t kipepeo_get_data_saved() {
    DataStats stats;
    data_tracker_get_stats(&stats);
    return stats.bytes_saved;
}

double kipepeo_get_compression_ratio() {
    DataStats stats;
    data_tracker_get_stats(&stats);
    return stats.compression_ratio;
}

void kipepeo_reset_stats() {
    LOGI("Resetting all statistics...");
    data_tracker_reset_stats();
    mediacodec_reset_stats();
}
