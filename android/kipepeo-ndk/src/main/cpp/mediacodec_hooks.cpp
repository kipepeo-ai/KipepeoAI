#include "mediacodec_hooks.h"
#include <android/log.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define TAG "KipepeoCodec"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

// Global statistics
static CodecStats g_stats = {0};
static pthread_mutex_t g_stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// Original function pointers (will be set by xhook)
static AMediaCodec* (*original_create_encoder)(const char*) = NULL;
static AMediaCodec* (*original_create_decoder)(const char*) = NULL;
static AVCodec* (*original_avcodec_find_encoder)(int) = NULL;
static AVCodec* (*original_avcodec_find_decoder)(int) = NULL;

bool mediacodec_hooks_init() {
    LOGI("Initializing MediaCodec hooks...");
    
    // Reset statistics
    pthread_mutex_lock(&g_stats_mutex);
    memset(&g_stats, 0, sizeof(CodecStats));
    pthread_mutex_unlock(&g_stats_mutex);
    
    LOGI("MediaCodec hooks initialized");
    return true;
}

void mediacodec_hooks_cleanup() {
    LOGI("Cleaning up MediaCodec hooks...");
    
    // Reset function pointers
    original_create_encoder = NULL;
    original_create_decoder = NULL;
    original_avcodec_find_encoder = NULL;
    original_avcodec_find_decoder = NULL;
    
    LOGI("MediaCodec hooks cleaned up");
}

/**
 * Check if MIME type is video codec
 */
static bool is_video_codec(const char* mime_type) {
    if (!mime_type) return false;
    
    return strstr(mime_type, "video/") != NULL;
}

/**
 * Convert MIME type to Kipepeo AV1
 */
static const char* convert_to_av1_mime(const char* mime_type) {
    if (!mime_type) return NULL;
    
    // If already AV1, return as-is
    if (strstr(mime_type, "av01") || strstr(mime_type, "av1")) {
        return mime_type;
    }
    
    // For video codecs, redirect to AV1
    if (is_video_codec(mime_type)) {
        LOGI("Converting MIME type '%s' to AV1", mime_type);
        return "video/av01";  // AV1 MIME type
    }
    
    // Non-video codecs pass through
    return mime_type;
}

AMediaCodec* hooked_create_encoder(const char* mime_type) {
    LOGD("hooked_create_encoder called with MIME: %s", mime_type ? mime_type : "NULL");
    
    // Update statistics
    pthread_mutex_lock(&g_stats_mutex);
    g_stats.encoders_intercepted++;
    pthread_mutex_unlock(&g_stats_mutex);
    
    // Convert to AV1 if it's a video codec
    const char* target_mime = convert_to_av1_mime(mime_type);
    
    if (target_mime != mime_type) {
        LOGI("Redirecting encoder from '%s' to '%s'", mime_type, target_mime);
    }
    
    // Call original function with potentially modified MIME type
    if (original_create_encoder) {
        return original_create_encoder(target_mime);
    }
    
    // Fallback: try to create AV1 encoder directly via dlsym
    LOGE("Original create_encoder not available, attempting fallback");
    return NULL;  // TODO: Implement fallback to core/video encoder
}

AMediaCodec* hooked_create_decoder(const char* mime_type) {
    LOGD("hooked_create_decoder called with MIME: %s", mime_type ? mime_type : "NULL");
    
    // Update statistics
    pthread_mutex_lock(&g_stats_mutex);
    g_stats.decoders_intercepted++;
    pthread_mutex_unlock(&g_stats_mutex);
    
    // Convert to AV1 if it's a video codec
    const char* target_mime = convert_to_av1_mime(mime_type);
    
    if (target_mime != mime_type) {
        LOGI("Redirecting decoder from '%s' to '%s'", mime_type, target_mime);
    }
    
    // Call original function with potentially modified MIME type
    if (original_create_decoder) {
        return original_create_decoder(target_mime);
    }
    
    // Fallback: try to create AV1 decoder directly via dlsym
    LOGE("Original create_decoder not available, attempting fallback");
    return NULL;  // TODO: Implement fallback to core/video decoder
}

// FFmpeg codec IDs (from libavcodec)
#define AV_CODEC_ID_H264 27
#define AV_CODEC_ID_HEVC 173
#define AV_CODEC_ID_VP8 139
#define AV_CODEC_ID_VP9 167
#define AV_CODEC_ID_AV1 226

/**
 * Check if codec ID should be redirected to AV1
 */
static bool should_redirect_ffmpeg_codec(int codec_id) {
    switch (codec_id) {
        case AV_CODEC_ID_H264:
        case AV_CODEC_ID_HEVC:
        case AV_CODEC_ID_VP8:
        case AV_CODEC_ID_VP9:
            return true;
        case AV_CODEC_ID_AV1:
            return false;  // Already AV1
        default:
            return false;  // Unknown/non-video codec
    }
}

AVCodec* hooked_avcodec_find_encoder(int codec_id) {
    LOGD("hooked_avcodec_find_encoder called with codec_id: %d", codec_id);
    
    // Update statistics
    pthread_mutex_lock(&g_stats_mutex);
    g_stats.encoders_intercepted++;
    pthread_mutex_unlock(&g_stats_mutex);
    
    // Redirect video codecs to AV1
    if (should_redirect_ffmpeg_codec(codec_id)) {
        LOGI("Redirecting FFmpeg encoder from codec_id %d to AV1 (226)", codec_id);
        codec_id = AV_CODEC_ID_AV1;
    }
    
    // Call original function
    if (original_avcodec_find_encoder) {
        return original_avcodec_find_encoder(codec_id);
    }
    
    LOGE("Original avcodec_find_encoder not available");
    return NULL;
}

AVCodec* hooked_avcodec_find_decoder(int codec_id) {
    LOGD("hooked_avcodec_find_decoder called with codec_id: %d", codec_id);
    
    // Update statistics
    pthread_mutex_lock(&g_stats_mutex);
    g_stats.decoders_intercepted++;
    pthread_mutex_unlock(&g_stats_mutex);
    
    // Redirect video codecs to AV1
    if (should_redirect_ffmpeg_codec(codec_id)) {
        LOGI("Redirecting FFmpeg decoder from codec_id %d to AV1 (226)", codec_id);
        codec_id = AV_CODEC_ID_AV1;
    }
    
    // Call original function
    if (original_avcodec_find_decoder) {
        return original_avcodec_find_decoder(codec_id);
    }
    
    LOGE("Original avcodec_find_decoder not available");
    return NULL;
}

void mediacodec_get_stats(CodecStats* stats) {
    if (!stats) return;
    
    pthread_mutex_lock(&g_stats_mutex);
    memcpy(stats, &g_stats, sizeof(CodecStats));
    pthread_mutex_unlock(&g_stats_mutex);
}

void mediacodec_reset_stats() {
    pthread_mutex_lock(&g_stats_mutex);
    memset(&g_stats, 0, sizeof(CodecStats));
    pthread_mutex_unlock(&g_stats_mutex);
    
    LOGI("Codec statistics reset");
}
