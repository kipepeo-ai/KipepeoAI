#include "kipepeo/video/rav1e_integration.h"
#include "kipepeo/video/av1_encoder.h"
#include <cstring>
#include <algorithm>

namespace kipepeo {
namespace video {

class Rav1eIntegration::Impl {
public:
    AV1Encoder encoder_;
    Rav1eConfig config_;
    EncodingStats stats_;
    uint64_t frame_count_ = 0;
    bool initialized_ = false;

    Impl() {
        memset(&stats_, 0, sizeof(stats_));
    }
};

Rav1eIntegration::Rav1eIntegration() : impl_(new Impl()) {}

Rav1eIntegration::~Rav1eIntegration() {
    delete impl_;
}

bool Rav1eIntegration::initialize(const Rav1eConfig& config) {
    impl_->config_ = config;

    // Convert Rav1eConfig to AV1Encoder::Config
    AV1Encoder::Config encoder_config;
    encoder_config.width = config.width;
    encoder_config.height = config.height;
    encoder_config.fps_num = config.fps_num;
    encoder_config.fps_den = config.fps_den;
    encoder_config.bitrate_kbps = config.bitrate_kbps;
    encoder_config.speed = 6; // Default speed preset
    encoder_config.threads = config.threads;
    encoder_config.use_kip_mode = config.use_kip_mode;
    encoder_config.kip_config = config.kip_config;
    encoder_config.low_latency = false; // Can be made configurable

    // Initialize encoder
    if (!impl_->encoder_.initialize(encoder_config)) {
        return false;
    }

    impl_->initialized_ = true;
    return true;
}

bool Rav1eIntegration::encode_frame(
    const uint8_t* yuv_frame,
    uint8_t* output,
    size_t output_size,
    size_t* bytes_written
) {
    if (!impl_->initialized_ || !yuv_frame || !output || !bytes_written) {
        return false;
    }

    // Prepare frame for encoding
    AV1Encoder::Frame frame;
    
    // YUV420 format: Y plane, then U, then V
    int width = impl_->config_.width;
    int height = impl_->config_.height;
    size_t y_size = width * height;
    size_t uv_size = (width / 2) * (height / 2);
    
    frame.y_plane = yuv_frame;
    frame.u_plane = yuv_frame + y_size;
    frame.v_plane = yuv_frame + y_size + uv_size;
    frame.y_stride = width;
    frame.uv_stride = width / 2;
    frame.width = width;
    frame.height = height;
    frame.pts = impl_->frame_count_;
    frame.force_keyframe = false;

    // Send frame to encoder
    if (!impl_->encoder_.send_frame(&frame)) {
        return false;
    }

    // Receive encoded packet
    AV1Encoder::Packet packet;
    if (!impl_->encoder_.receive_packet(&packet)) {
        // May need more frames before packet is available
        *bytes_written = 0;
        return true; // Not an error, just need more data
    }

    // Copy packet data to output
    if (packet.size > output_size) {
        return false; // Output buffer too small
    }
    
    memcpy(output, packet.data, packet.size);
    *bytes_written = packet.size;

    // Update stats
    impl_->frame_count_++;
    impl_->stats_.total_frames++;
    impl_->stats_.total_bytes += packet.size;
    impl_->stats_.average_bitrate = 
        (impl_->stats_.total_bytes * 8.0f / 1000.0f) / 
        (impl_->frame_count_ * impl_->config_.fps_den / 
         static_cast<float>(impl_->config_.fps_num));

    return true;
}

bool Rav1eIntegration::flush(uint8_t* output, size_t output_size, size_t* bytes_written) {
    if (!impl_->initialized_ || !output || !bytes_written) {
        return false;
    }

    // Flush encoder
    AV1Encoder::Packet packet;
    if (!impl_->encoder_.flush(&packet)) {
        *bytes_written = 0;
        return true; // No more packets
    }

    // Copy packet data
    if (packet.size > output_size) {
        return false;
    }
    
    memcpy(output, packet.data, packet.size);
    *bytes_written = packet.size;

    return true;
}

Rav1eIntegration::EncodingStats Rav1eIntegration::get_stats() const {
    EncodingStats stats = impl_->stats_;
    
    // Get encoder stats
    AV1Encoder::Stats encoder_stats = impl_->encoder_.get_stats();
    stats.average_psnr = encoder_stats.average_psnr;
    
    if (impl_->config_.use_kip_mode) {
        stats.kip_stats = encoder_stats.kip_stats;
    }
    
    return stats;
}

} // namespace video
} // namespace kipepeo
