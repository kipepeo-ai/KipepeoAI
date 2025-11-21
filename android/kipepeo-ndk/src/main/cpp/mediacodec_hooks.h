#ifndef KIPEPEO_MEDIACODEC_HOOKS_H
#define KIPEPEO_MEDIACODEC_HOOKS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for MediaCodec types
struct AMediaCodec;
typedef struct AMediaCodec AMediaCodec;

// Forward declarations for FFmpeg types
struct AVCodec;
typedef struct AVCodec AVCodec;

/**
 * Initialize MediaCodec hooking subsystem
 * 
 * @return true if successful, false otherwise
 */
bool mediacodec_hooks_init();

/**
 * Cleanup MediaCodec hooks
 */
void mediacodec_hooks_cleanup();

/**
 * Hook function for AMediaCodec_createEncoderByType
 * 
 * Intercepts encoder creation and redirects to Kipepeo's AV1 encoder
 */
AMediaCodec* hooked_create_encoder(const char* mime_type);

/**
 * Hook function for AMediaCodec_createDecoderByType
 * 
 * Intercepts decoder creation and redirects to Kipepeo's AV1 decoder
 */
AMediaCodec* hooked_create_decoder(const char* mime_type);

/**
 * Hook function for avcodec_find_encoder (FFmpeg)
 * 
 * Intercepts FFmpeg encoder lookup
 */
AVCodec* hooked_avcodec_find_encoder(int codec_id);

/**
 * Hook function for avcodec_find_decoder (FFmpeg)
 * 
 * Intercepts FFmpeg decoder lookup
 */
AVCodec* hooked_avcodec_find_decoder(int codec_id);

/**
 * Get statistics about codec usage
 */
typedef struct {
    uint64_t encoders_intercepted;
    uint64_t decoders_intercepted;
    uint64_t bytes_encoded;
    uint64_t bytes_decoded;
    uint64_t original_bytes;  // Estimated original size
    uint64_t compressed_bytes;  // Actual AV1 compressed size
} CodecStats;

/**
 * Get current codec statistics
 */
void mediacodec_get_stats(CodecStats* stats);

/**
 * Reset statistics
 */
void mediacodec_reset_stats();

#ifdef __cplusplus
}
#endif

#endif // KIPEPEO_MEDIACODEC_HOOKS_H
