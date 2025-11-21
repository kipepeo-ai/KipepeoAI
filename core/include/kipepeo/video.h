#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Kipepeo Video Encoding/Decoding C API
 * 
 * Clean C interface for AV1 video operations with:
 * - kip-mode-lowband: Custom rate control using on-device CLIP
 * - African skin tone preservation
 * - Talking head detection and adaptive bitrate allocation
 * - Real-time encoding on low-end mobile devices
 */

// Opaque handle types
typedef struct kipepeo_video_encoder kipepeo_video_encoder_t;
typedef struct kipepeo_video_decoder kipepeo_video_decoder_t;

// Error codes
typedef enum {
    KIPEPEO_VIDEO_SUCCESS = 0,
    KIPEPEO_VIDEO_ERROR_INVALID_PARAM = -1,
    KIPEPEO_VIDEO_ERROR_OUT_OF_MEMORY = -2,
    KIPEPEO_VIDEO_ERROR_ENCODE_FAILED = -3,
    KIPEPEO_VIDEO_ERROR_DECODE_FAILED = -4,
    KIPEPEO_VIDEO_ERROR_UNSUPPORTED_FORMAT = -5,
} kipepeo_video_error_t;

// Pixel formats
typedef enum {
    KIPEPEO_PIX_FMT_YUV420P = 0,  // Planar YUV 4:2:0
    KIPEPEO_PIX_FMT_NV12 = 1,     // Semi-planar YUV 4:2:0
    KIPEPEO_PIX_FMT_NV21 = 2,     // Semi-planar YUV 4:2:0 (Android camera format)
} kipepeo_pixel_format_t;

// Rate control modes
typedef enum {
    KIPEPEO_RC_CQP = 0,           // Constant Quantization Parameter
    KIPEPEO_RC_CBR = 1,           // Constant Bitrate
    KIPEPEO_RC_VBR = 2,           // Variable Bitrate
    KIPEPEO_RC_KIP_LOWBAND = 100, // kip-mode-lowband (AI-driven, skin-tone aware)
} kipepeo_rate_control_mode_t;

// Video frame
typedef struct {
    uint8_t* data[3];             // Plane pointers (Y, U, V or Y, UV for semi-planar)
    int32_t linesize[3];          // Line sizes for each plane
    uint32_t width;               // Frame width
    uint32_t height;              // Frame height
    kipepeo_pixel_format_t format; // Pixel format
    int64_t pts;                  // Presentation timestamp
} kipepeo_video_frame_t;

// Encoder parameters
typedef struct {
    uint32_t width;                        // Output width
    uint32_t height;                       // Output height
    uint32_t fps_num;                      // Framerate numerator (e.g., 30)
    uint32_t fps_den;                      // Framerate denominator (e.g., 1)
    kipepeo_rate_control_mode_t rc_mode;   // Rate control mode
    uint32_t bitrate;                      // Target bitrate in kbps (for CBR/VBR)
    uint32_t qp;                           // Quantization parameter (for CQP, 0-255)
    uint32_t keyframe_interval;            // Keyframe interval in frames (0 = auto)
    uint32_t threads;                      // Number of threads (0 = auto)
    bool low_latency;                      // Low latency mode (default: false)
    bool tune_for_mobile;                  // Mobile device optimizations (default: true)
    
    // kip-mode-lowband specific parameters
    bool enable_skin_tone_protection;      // Protect African skin tones (default: true)
    bool enable_talking_head_detection;    // Detect and prioritize talking heads (default: true)
    bool enable_clip_analysis;             // Use on-device CLIP for scene analysis (default: true)
    float skin_tone_boost;                 // Quality boost for skin regions (1.0-2.0, default: 1.5)
} kipepeo_encoder_params_t;

// Decoder parameters
typedef struct {
    uint32_t threads;             // Number of threads (0 = auto)
    bool low_latency;             // Low latency mode (default: false)
} kipepeo_decoder_params_t;

// Encoding statistics (per frame)
typedef struct {
    uint64_t frame_number;        // Frame number
    size_t compressed_size;       // Compressed frame size in bytes
    float psnr;                   // PSNR (if available)
    float ssim;                   // SSIM (if available)
    uint32_t encoding_time_ms;    // Encoding time in milliseconds
    bool is_keyframe;             // True if keyframe
    
    // kip-mode-lowband specific stats
    float skin_tone_coverage;     // Percentage of frame with detected skin tones
    float talking_head_score;     // Confidence score for talking head detection (0.0-1.0)
    uint32_t allocated_bitrate;   // Actual allocated bitrate for this frame
} kipepeo_encode_stats_t;

// Callback for encoding statistics
typedef void (*kipepeo_encode_stats_callback_t)(const kipepeo_encode_stats_t* stats, void* user_data);

/**
 * Initialize Kipepeo video library
 * Must be called once before using any other functions
 */
kipepeo_video_error_t kipepeo_video_init(void);

/**
 * Cleanup Kipepeo video library
 */
void kipepeo_video_cleanup(void);

/**
 * Get default encoder parameters
 */
kipepeo_encoder_params_t kipepeo_encoder_params_default(void);

/**
 * Get default decoder parameters
 */
kipepeo_decoder_params_t kipepeo_decoder_params_default(void);

/**
 * Create encoder instance
 * 
 * @param params Encoder parameters
 * @param encoder Output encoder handle
 * @return Error code
 */
kipepeo_video_error_t kipepeo_encoder_create(
    const kipepeo_encoder_params_t* params,
    kipepeo_video_encoder_t** encoder
);

/**
 * Free encoder instance
 */
void kipepeo_encoder_free(kipepeo_video_encoder_t* encoder);

/**
 * Encode a video frame
 * 
 * @param encoder Encoder handle
 * @param frame Input frame (NULL to flush encoder)
 * @param output Output buffer (allocated by caller)
 * @param output_size Size of output buffer
 * @param bytes_written Number of bytes written to output
 * @return Error code
 */
kipepeo_video_error_t kipepeo_encoder_encode(
    kipepeo_video_encoder_t* encoder,
    const kipepeo_video_frame_t* frame,
    uint8_t* output,
    size_t output_size,
    size_t* bytes_written
);

/**
 * Encode a frame with statistics callback
 */
kipepeo_video_error_t kipepeo_encoder_encode_with_stats(
    kipepeo_video_encoder_t* encoder,
    const kipepeo_video_frame_t* frame,
    uint8_t* output,
    size_t output_size,
    size_t* bytes_written,
    kipepeo_encode_stats_callback_t stats_callback,
    void* user_data
);

/**
 * Force keyframe on next encode
 */
void kipepeo_encoder_force_keyframe(kipepeo_video_encoder_t* encoder);

/**
 * Update encoder bitrate dynamically (for adaptive streaming)
 */
kipepeo_video_error_t kipepeo_encoder_set_bitrate(
    kipepeo_video_encoder_t* encoder,
    uint32_t bitrate_kbps
);

/**
 * Create decoder instance
 */
kipepeo_video_error_t kipepeo_decoder_create(
    const kipepeo_decoder_params_t* params,
    kipepeo_video_decoder_t** decoder
);

/**
 * Free decoder instance
 */
void kipepeo_decoder_free(kipepeo_video_decoder_t* decoder);

/**
 * Decode compressed data to frame
 * 
 * @param decoder Decoder handle
 * @param input Input compressed data
 * @param input_size Size of input data
 * @param frame Output frame (allocated by library, user must call kipepeo_frame_free)
 * @return Error code
 */
kipepeo_video_error_t kipepeo_decoder_decode(
    kipepeo_video_decoder_t* decoder,
    const uint8_t* input,
    size_t input_size,
    kipepeo_video_frame_t** frame
);

/**
 * Free a decoded frame
 */
void kipepeo_frame_free(kipepeo_video_frame_t* frame);

/**
 * Get last error message
 */
const char* kipepeo_video_get_error_string(kipepeo_video_error_t error);

/**
 * Get library version string
 */
const char* kipepeo_video_get_version(void);

/**
 * Check if NEON optimizations are enabled for video
 */
bool kipepeo_video_has_neon_support(void);

/**
 * Get estimated bitrate savings vs standard AV1 (based on kip-mode-lowband)
 * Returns: Percentage savings (e.g., 0.45 = 45%)
 */
float kipepeo_video_get_bitrate_savings_estimate(void);

#ifdef __cplusplus
}
#endif
