#pragma once

#include "kipepeo/video/kip_mode_lowband.h"
#include <string>

// Forward declarations (rav1e C API would be included here)
// extern "C" {
//     struct RaContext;
//     struct RaConfig;
// }

namespace kipepeo {
namespace video {

/**
 * Integration layer for rav1e with kip-mode-lowband rate control
 * 
 * This wrapper extends rav1e to support:
 * - Custom rate control via kip-mode-lowband
 * - African skin tone-aware encoding
 * - Mobile device optimizations
 */

struct Rav1eConfig {
    uint32_t width;
    uint32_t height;
    uint32_t fps_num;
    uint32_t fps_den;
    uint32_t bitrate_kbps;
    uint32_t threads;
    bool use_kip_mode;              // Enable kip-mode-lowband
    RateControlConfig kip_config;   // kip-mode configuration
};

class Rav1eIntegration {
public:
    Rav1eIntegration();
    ~Rav1eIntegration();

    /**
     * Initialize encoder with configuration
     */
    bool initialize(const Rav1eConfig& config);

    /**
     * Encode a YUV420 frame
     * 
     * @param yuv_frame YUV420 frame data
     * @param output Output buffer for compressed data
     * @param output_size Size of output buffer
     * @param bytes_written Actual bytes written
     * @return true on success
     */
    bool encode_frame(
        const uint8_t* yuv_frame,
        uint8_t* output,
        size_t output_size,
        size_t* bytes_written
    );

    /**
     * Flush encoder
     */
    bool flush(uint8_t* output, size_t output_size, size_t* bytes_written);

    /**
     * Get encoding statistics
     */
    struct EncodingStats {
        uint64_t total_frames;
        uint64_t total_bytes;
        float average_psnr;
        float average_bitrate;
        KipModeLowband::Stats kip_stats;
    };

    EncodingStats get_stats() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace video
} // namespace kipepeo
