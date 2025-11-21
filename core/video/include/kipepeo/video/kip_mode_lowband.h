#pragma once

#include <cstdint.h>
#include <cstddef.h>

namespace kipepeo {
namespace video {

/**
 * kip-mode-lowband: AI-driven AV1 rate control
 * 
 * Custom rate control algorithm that uses on-device CLIP to:
 * - Detect and protect African skin tones
 * - Identify talking heads and prioritize face regions
 * - Adaptively allocate bitrate based on perceptual importance
 * - Achieve 40-50% bitrate savings at same perceived quality
 */

struct FrameAnalysis {
    float skin_tone_coverage;      // Percentage of frame with skin tones (0.0-1.0)
    float talking_head_score;      // Confidence for talking head detection (0.0-1.0)
    float scene_complexity;        // Scene complexity score (0.0-1.0)
    float temporal_stability;      // How similar to previous frame (0.0-1.0)
    bool is_keyframe_needed;       // Whether to force keyframe
    
    // Detected regions of interest
    struct ROI {
        uint16_t x, y, width, height;
        float importance;          // Perceptual importance (0.0-1.0)
    } regions[8];                  // Up to 8 ROIs
    uint32_t num_regions;
};

struct RateControlConfig {
    uint32_t target_bitrate_kbps;  // Target bitrate
    float skin_tone_boost;         // Quality boost for skin regions (1.0-2.0)
    bool enable_skin_protection;   // Enable skin tone preservation
    bool enable_talking_head_detection;
    bool enable_clip_analysis;     // Use CLIP for frame analysis
    float min_qp_skin;            // Min QP for skin regions
    float max_qp_background;      // Max QP for background
};

class KipModeLowband {
public:
    KipModeLowband();
    ~KipModeLowband();

    /**
     * Initialize rate controller with configuration
     */
    bool initialize(const RateControlConfig& config);

    /**
     * Analyze a frame using on-device CLIP
     * 
     * @param yuv_frame YUV420 frame data
     * @param width Frame width
     * @param height Frame height
     * @param analysis Output frame analysis
     * @return true on success
     */
    bool analyze_frame(
        const uint8_t* yuv_frame,
        uint32_t width,
        uint32_t height,
        FrameAnalysis* analysis
    );

    /**
     * Compute QP (quantization parameter) for a macroblock/CTU
     * 
     * @param mb_x Macroblock X position
     * @param mb_y Macroblock Y position
     * @param base_qp Base QP for frame
     * @param analysis Frame analysis data
     * @return Adjusted QP for this macroblock
     */
    float compute_macroblock_qp(
        uint32_t mb_x,
        uint32_t mb_y,
        float base_qp,
        const FrameAnalysis* analysis
    );

    /**
     * Compute target bitrate for current frame based on analysis
     */
    uint32_t compute_frame_bitrate(
        const FrameAnalysis* analysis,
        uint32_t base_bitrate
    );

    /**
     * Update rate control state after encoding a frame
     * 
     * @param actual_bits Actual bits used for the frame
     * @param target_bits Target bits for the frame
     */
    void update_after_frame(uint32_t actual_bits, uint32_t target_bits);

    /**
     * Get statistics for debugging/logging
     */
    struct Stats {
        uint64_t total_frames;
        uint64_t total_bits;
        float average_skin_coverage;
        float average_bitrate_savings;
    };

    Stats get_stats() const;

private:
    class Impl;
    Impl* impl_;
};

/**
 * Skin tone detector (African skin tones specific)
 * Uses YUV color space analysis optimized for diverse African skin tones
 */
class SkinToneDetector {
public:
    SkinToneDetector();
    ~SkinToneDetector();

    /**
     * Detect skin pixels in YUV frame
     * 
     * @param yuv_frame YUV420 frame data 
     * @param width Frame width
     * @param height Frame height
     * @param skin_mask Output binary mask (1=skin, 0=non-skin)
     * @return Percentage of skin pixels (0.0-1.0)
     */
    float detect_skin(
        const uint8_t* yuv_frame,
        uint32_t width,
        uint32_t height,
        uint8_t* skin_mask
    );

    /**
     * Calibrate detector for specific skin tone range
     * (Pre-trained for African skin tones)
     */
    void calibrate_for_african_skin_tones();

private:
    class Impl;
    Impl* impl_;
};

/**
 * Lightweight CLIP model for on-device scene analysis
 * Quantized to ~10MB for mobile deployment
 */
class MobileCLIP {
public:
    MobileCLIP();
    ~MobileCLIP();

    /**
     * Load quantized CLIP model from file
     */
    bool load_model(const char* model_path);

    /**
     * Analyze frame and extract semantic features
     * Returns features useful for rate control decisions
     */
    bool analyze(
        const uint8_t* rgb_frame,
        uint32_t width,
        uint32_t height,
        float* features,      // Output feature vector (512 dims)
        size_t feature_dim
    );

    /**
     * Detect if frame contains a talking head
     * Returns confidence score 0.0-1.0
     */
    float detect_talking_head(const uint8_t* rgb_frame, uint32_t width, uint32_t height);

private:
    class Impl;
    Impl* impl_;
};

} // namespace video
} // namespace kipepeo
