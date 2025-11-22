#pragma once

#include "kipepeo/video/kip_mode_lowband.h"
#include "kipepeo/video/types.h"
#include <cstdint>
#include <cstddef>

namespace kipepeo {
namespace video {

/**
 * Rate Control - Bridge between VideoCompressor and KipModeLowband
 * 
 * Provides adaptive bitrate adjustment based on frame analysis
 */
class RateControl {
public:
    RateControl();
    ~RateControl();

    /**
     * Rate control configuration
     */
    struct Config {
        uint32_t target_bitrate_kbps;      // Target bitrate
        uint32_t min_bitrate_kbps;         // Minimum bitrate
        uint32_t max_bitrate_kbps;         // Maximum bitrate
        float bitrate_variance;            // Allowed variance (0.0-1.0)
        bool enable_kip_mode;             // Enable kip-mode-lowband
        RateControlConfig kip_config;      // kip-mode configuration
    };

    /**
     * Initialize rate controller
     * 
     * @param config Rate control configuration
     * @return true on success
     */
    bool initialize(const Config& config);

    /**
     * Analyze frame and compute target bitrate
     * 
     * @param yuv_frame YUV420 frame data
     * @param width Frame width
     * @param height Frame height
     * @param base_bitrate Base bitrate for frame
     * @param target_bitrate Output target bitrate
     * @param analysis Output frame analysis (optional)
     * @return true on success
     */
    bool compute_target_bitrate(
        const uint8_t* yuv_frame,
        uint32_t width,
        uint32_t height,
        uint32_t base_bitrate,
        uint32_t* target_bitrate,
        FrameAnalysis* analysis = nullptr
    );

    /**
     * Update rate control after encoding frame
     * 
     * @param actual_bits Actual bits used for frame
     * @param target_bits Target bits for frame
     */
    void update_after_frame(uint32_t actual_bits, uint32_t target_bits);

    /**
     * Get current bitrate adjustment factor
     * 
     * @return Adjustment factor (1.0 = no adjustment, >1.0 = increase, <1.0 = decrease)
     */
    float get_bitrate_adjustment() const;

    /**
     * Get rate control statistics
     */
    struct Stats {
        uint64_t total_frames;
        uint64_t total_bits;
        float average_bitrate_kbps;
        float bitrate_savings_percent;
        KipModeLowband::Stats kip_stats;
    };

    Stats get_stats() const;

private:
    class Impl;
    class Impl* impl_;
};

} // namespace video
} // namespace kipepeo

