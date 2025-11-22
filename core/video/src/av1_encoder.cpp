#include "kipepeo/video/av1_encoder.h"
#include "kipepeo/video/rav1e_capi.h"
#include "kipepeo/video/kip_mode_lowband.h"
#include <cstring>
#include <chrono>
#include <functional>

namespace kipepeo {
namespace video {

class AV1Encoder::Impl {
public:
    RaContext* ctx_ = nullptr;
    RaConfig* config_ = nullptr;
    Config encoder_config_;
    Stats stats_;
    bool initialized_ = false;
    
    // kip-mode-lowband integration
    KipModeLowband kip_mode_;
    bool use_kip_mode_ = false;
    
    // QP callback for per-macroblock adjustment
    std::function<float(uint32_t, uint32_t, float)> qp_callback_;
    
    // Frame timing
    std::chrono::high_resolution_clock::time_point encode_start_;
    uint64_t frame_count_ = 0;
    
    Impl() {
        memset(&stats_, 0, sizeof(stats_));
    }
    
    ~Impl() {
        cleanup();
    }
    
    void cleanup() {
        if (ctx_) {
            rav1e_context_unref(ctx_);
            ctx_ = nullptr;
        }
        if (config_) {
            rav1e_config_unref(config_);
            config_ = nullptr;
        }
    }
};

AV1Encoder::AV1Encoder() : impl_(std::make_unique<Impl>()) {}

AV1Encoder::~AV1Encoder() = default;

bool AV1Encoder::initialize(const Config& config) {
    if (impl_->initialized_) {
        impl_->cleanup();
    }
    
    impl_->encoder_config_ = config;
    
    // Create default config
    impl_->config_ = rav1e_config_default();
    if (!impl_->config_) {
        return false;
    }
    
    // Set dimensions using parse
    char width_str[16], height_str[16];
    snprintf(width_str, sizeof(width_str), "%d", config.width);
    snprintf(height_str, sizeof(height_str), "%d", config.height);
    if (rav1e_config_parse(impl_->config_, "width", width_str) < 0 ||
        rav1e_config_parse(impl_->config_, "height", height_str) < 0) {
        rav1e_config_unref(impl_->config_);
        impl_->config_ = nullptr;
        return false;
    }
    
    // Set time base (fps)
    RaRational time_base = { config.fps_den, config.fps_num };
    if (rav1e_config_set_time_base(impl_->config_, time_base) < 0) {
        rav1e_config_unref(impl_->config_);
        impl_->config_ = nullptr;
        return false;
    }
    
    // Set pixel format (YUV420, 8-bit)
    if (rav1e_config_set_pixel_format(impl_->config_, 8, 
                                       RA_CHROMA_SAMPLING_420,
                                       0, RA_PIXEL_RANGE_LIMITED) < 0) {
        rav1e_config_unref(impl_->config_);
        impl_->config_ = nullptr;
        return false;
    }
    
    // Set speed preset
    char speed_str[16];
    snprintf(speed_str, sizeof(speed_str), "%d", config.speed);
    rav1e_config_parse(impl_->config_, "speed", speed_str);
    
    // Set bitrate
    char bitrate_str[32];
    snprintf(bitrate_str, sizeof(bitrate_str), "%d", config.bitrate_kbps);
    rav1e_config_parse(impl_->config_, "bitrate", bitrate_str);
    
    // Set threads
    if (config.threads > 0) {
        char threads_str[16];
        snprintf(threads_str, sizeof(threads_str), "%d", config.threads);
        rav1e_config_parse(impl_->config_, "threads", threads_str);
    }
    
    // Low-latency mode
    if (config.low_latency) {
        rav1e_config_parse_int(impl_->config_, "low_latency", 1);
    }
    
    // Create context
    impl_->ctx_ = rav1e_context_new(impl_->config_);
    if (!impl_->ctx_) {
        rav1e_config_unref(impl_->config_);
        impl_->config_ = nullptr;
        return false;
    }
    
    // Initialize kip-mode if enabled
    if (config.use_kip_mode) {
        impl_->use_kip_mode_ = true;
        if (!impl_->kip_mode_.initialize(config.kip_config)) {
            impl_->cleanup();
            return false;
        }
    }
    
    impl_->initialized_ = true;
    return true;
}

bool AV1Encoder::send_frame(const Frame* frame) {
    if (!impl_->initialized_) {
        return false;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create new frame
    RaFrame* ra_frame = rav1e_frame_new(impl_->ctx_);
    if (!ra_frame) {
        return false;
    }
    
    if (frame) {
        // Fill Y plane
        size_t y_size = frame->width * frame->height;
        rav1e_frame_fill_plane(ra_frame, 0, frame->y_plane, 
                               y_size, frame->y_stride, 1);
        
        // Fill U plane
        int uv_width = frame->width / 2;
        int uv_height = frame->height / 2;
        size_t uv_size = uv_width * uv_height;
        rav1e_frame_fill_plane(ra_frame, 1, frame->u_plane,
                               uv_size, frame->uv_stride, 1);
        
        // Fill V plane
        rav1e_frame_fill_plane(ra_frame, 2, frame->v_plane,
                               uv_size, frame->uv_stride, 1);
        
        // Force keyframe if requested
        if (frame->force_keyframe) {
            rav1e_frame_set_type(ra_frame, RA_FRAME_TYPE_OVERRIDE_KEY);
        }
        
        // Analyze frame with kip-mode if enabled
        if (impl_->use_kip_mode_) {
            FrameAnalysis analysis;
            // Create temporary YUV buffer for analysis
            size_t y_size = frame->width * frame->height;
            size_t uv_size = uv_width * uv_height;
            uint8_t* yuv_buffer = new uint8_t[y_size + uv_size * 2];
            
            // Copy Y plane
            memcpy(yuv_buffer, frame->y_plane, y_size);
            // Copy U plane
            memcpy(yuv_buffer + y_size, frame->u_plane, uv_size);
            // Copy V plane
            memcpy(yuv_buffer + y_size + uv_size, frame->v_plane, uv_size);
            
            impl_->kip_mode_.analyze_frame(yuv_buffer, frame->width, frame->height, &analysis);
            
            delete[] yuv_buffer;
            
            // Use analysis to adjust encoding (would need rav1e API support)
            // For now, we store the analysis for statistics
        }
    }
    
    // Send frame to encoder
    RaEncoderStatus status = rav1e_send_frame(impl_->ctx_, frame ? ra_frame : nullptr);
    
    if (frame) {
        rav1e_frame_unref(ra_frame);
    }
    
    if (status == RA_ENCODER_STATUS_FAILURE || status == RA_ENCODER_STATUS_NOT_READY) {
        return false;
    }
    
    // Update stats
    auto end_time = std::chrono::high_resolution_clock::now();
    auto encode_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time).count() / 1000.0f;
    
    if (frame) {
        impl_->frame_count_++;
        impl_->stats_.average_encoding_time_ms = 
            (impl_->stats_.average_encoding_time_ms * (impl_->frame_count_ - 1) + 
             encode_time) / impl_->frame_count_;
    }
    
    return true;
}

bool AV1Encoder::receive_packet(Packet* packet) {
    if (!impl_->initialized_ || !packet) {
        return false;
    }
    
    RaPacket* ra_packet = nullptr;
    RaEncoderStatus status = rav1e_receive_packet(impl_->ctx_, &ra_packet);
    
    if (status != RA_ENCODER_STATUS_SUCCESS || !ra_packet) {
        return false;
    }
    
    // Extract packet data
    packet->data = rav1e_packet_data(ra_packet);
    packet->size = rav1e_packet_len(ra_packet);
    packet->frame_number = rav1e_packet_input_frameno(ra_packet);
    packet->pts = 0; // PTS not directly available
    packet->is_keyframe = (rav1e_packet_frame_type(ra_packet) == 0); // Simplified
    
    // Update stats
    impl_->stats_.frames_encoded++;
    impl_->stats_.bytes_encoded += packet->size;
    impl_->stats_.average_bitrate_kbps = 
        (impl_->stats_.bytes_encoded * 8.0f / 1000.0f) / 
        (impl_->frame_count_ * impl_->encoder_config_.fps_den / 
         static_cast<float>(impl_->encoder_config_.fps_num));
    
    // Note: ra_packet is owned by rav1e, we don't unref it here
    // The caller should copy the data if needed
    
    return true;
}

bool AV1Encoder::flush(Packet* packet) {
    // Send nullptr frame to flush
    if (!send_frame(nullptr)) {
        return false;
    }
    
    // Receive all buffered packets
    return receive_packet(packet);
}

AV1Encoder::Stats AV1Encoder::get_stats() const {
    Stats stats = impl_->stats_;
    if (impl_->use_kip_mode_) {
        stats.kip_stats = impl_->kip_mode_.get_stats();
    }
    return stats;
}

bool AV1Encoder::is_hardware_available() {
    // Check for hardware acceleration
    // On Android: Check MediaCodec AV1 support
    // On iOS: Check VideoToolbox AV1 support
    // For now, return false (software only)
    return false;
}

void AV1Encoder::set_qp_callback(std::function<float(uint32_t, uint32_t, float)> callback) {
    impl_->qp_callback_ = callback;
}

} // namespace video
} // namespace kipepeo
