#include "kipepeo/video/rate_control.h"
#include "kipepeo/video/kip_mode_lowband.h"
#include <cstring>
#include <algorithm>

namespace kipepeo {
namespace video {

class RateControl::Impl {
public:
    Config config_;
    KipModeLowband kip_mode_;
    Stats stats_;
    bool initialized_ = false;
    
    // Bitrate adjustment state
    float bitrate_adjustment_ = 1.0f;
    float bitrate_error_ = 0.0f; // Accumulated error
    
    Impl() {
        memset(&stats_, 0, sizeof(stats_));
    }
};

RateControl::RateControl() : impl_(new Impl()) {}

RateControl::~RateControl() {
    delete impl_;
}

bool RateControl::initialize(const Config& config) {
    impl_->config_ = config;
    
    // Initialize kip-mode if enabled
    if (config.enable_kip_mode) {
        if (!impl_->kip_mode_.initialize(config.kip_config)) {
            return false;
        }
    }
    
    impl_->initialized_ = true;
    return true;
}

bool RateControl::compute_target_bitrate(
    const uint8_t* yuv_frame,
    uint32_t width,
    uint32_t height,
    uint32_t base_bitrate,
    uint32_t* target_bitrate,
    FrameAnalysis* analysis
) {
    if (!impl_->initialized_ || !yuv_frame || !target_bitrate) {
        return false;
    }
    
    FrameAnalysis frame_analysis;
    FrameAnalysis* analysis_ptr = analysis ? analysis : &frame_analysis;
    
    // Analyze frame with kip-mode if enabled
    if (impl_->config_.enable_kip_mode) {
        if (!impl_->kip_mode_.analyze_frame(yuv_frame, width, height, analysis_ptr)) {
            return false;
        }
        
        // Compute target bitrate based on analysis
        *target_bitrate = impl_->kip_mode_.compute_frame_bitrate(analysis_ptr, base_bitrate);
    } else {
        // No analysis, use base bitrate with adjustment
        *target_bitrate = static_cast<uint32_t>(base_bitrate * impl_->bitrate_adjustment_);
    }
    
    // Apply min/max constraints
    *target_bitrate = std::max(impl_->config_.min_bitrate_kbps,
                               std::min(impl_->config_.max_bitrate_kbps, *target_bitrate));
    
    // Apply variance constraint
    float variance_factor = 1.0f + impl_->config_.bitrate_variance * impl_->bitrate_error_;
    *target_bitrate = static_cast<uint32_t>(*target_bitrate * variance_factor);
    
    // Update stats
    impl_->stats_.total_frames++;
    
    return true;
}

void RateControl::update_after_frame(uint32_t actual_bits, uint32_t target_bits) {
    if (!impl_->initialized_) {
        return;
    }
    
    impl_->stats_.total_bits += actual_bits;
    
    // Update kip-mode if enabled
    if (impl_->config_.enable_kip_mode) {
        impl_->kip_mode_.update_after_frame(actual_bits, target_bits);
    }
    
    // Update bitrate error (for variance adjustment)
    if (target_bits > 0) {
        float error = (static_cast<float>(actual_bits) - static_cast<float>(target_bits)) /
                      static_cast<float>(target_bits);
        impl_->bitrate_error_ = 0.9f * impl_->bitrate_error_ + 0.1f * error;
    }
    
    // Update bitrate adjustment
    if (target_bits > 0) {
        float ratio = static_cast<float>(actual_bits) / static_cast<float>(target_bits);
        impl_->bitrate_adjustment_ = 0.95f * impl_->bitrate_adjustment_ + 0.05f * (1.0f / ratio);
        
        // Clamp adjustment
        impl_->bitrate_adjustment_ = std::max(0.5f, std::min(2.0f, impl_->bitrate_adjustment_));
    }
}

float RateControl::get_bitrate_adjustment() const {
    return impl_->bitrate_adjustment_;
}

RateControl::Stats RateControl::get_stats() const {
    Stats stats = impl_->stats_;
    
    // Calculate average bitrate
    if (stats.total_frames > 0) {
        stats.average_bitrate_kbps = 
            (stats.total_bits * 8.0f / 1000.0f) / stats.total_frames;
    }
    
    // Calculate bitrate savings (compared to base bitrate)
    if (impl_->config_.target_bitrate_kbps > 0 && stats.total_frames > 0) {
        float base_bits = impl_->config_.target_bitrate_kbps * 1000.0f / 8.0f * stats.total_frames;
        if (base_bits > 0) {
            stats.bitrate_savings_percent = 
                (1.0f - static_cast<float>(stats.total_bits) / base_bits) * 100.0f;
        }
    }
    
    // Get kip-mode stats if enabled
    if (impl_->config_.enable_kip_mode) {
        stats.kip_stats = impl_->kip_mode_.get_stats();
    }
    
    return stats;
}

} // namespace video
} // namespace kipepeo
