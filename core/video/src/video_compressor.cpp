#include "kipepeo/video/video_compressor.h"
#include "kipepeo/video/av1_encoder.h"
#include "kipepeo/video/rate_control.h"
#include "kipepeo/video/types.h"
#include <cstring>
#include <algorithm>

namespace kipepeo {
namespace video {

class VideoCompressor::Impl {
public:
    AV1Encoder encoder_;
    RateControl rate_control_;
    EncodingParams params_;
    CompressionStats stats_;
    bool initialized_ = false;
    bool ai_rate_control_enabled_ = true;
    
    // Format conversion buffers
    uint8_t* yuv_buffer_ = nullptr;
    size_t yuv_buffer_size_ = 0;
    
    Impl() {
        memset(&stats_, 0, sizeof(stats_));
    }
    
    ~Impl() {
        cleanup();
    }
    
    void cleanup() {
        if (yuv_buffer_) {
            delete[] yuv_buffer_;
            yuv_buffer_ = nullptr;
        }
        yuv_buffer_size_ = 0;
    }
    
    bool ensure_yuv_buffer(size_t size) {
        if (yuv_buffer_size_ < size) {
            delete[] yuv_buffer_;
            yuv_buffer_ = new uint8_t[size];
            if (!yuv_buffer_) {
                return false;
            }
            yuv_buffer_size_ = size;
        }
        return true;
    }
    
    // Convert RGB to YUV420
    void rgb_to_yuv420(const uint8_t* rgb, int width, int height, 
                       uint8_t* yuv) {
        size_t y_size = width * height;
        uint8_t* y_plane = yuv;
        uint8_t* u_plane = yuv + y_size;
        uint8_t* v_plane = yuv + y_size + (width / 2) * (height / 2);
        
        // Simple RGB to YUV conversion
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                uint8_t r = rgb[idx];
                uint8_t g = rgb[idx + 1];
                uint8_t b = rgb[idx + 2];
                
                // Y = 0.299*R + 0.587*G + 0.114*B
                y_plane[y * width + x] = 
                    static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
                
                // U and V (subsampled)
                if (x % 2 == 0 && y % 2 == 0) {
                    int uv_idx = (y / 2) * (width / 2) + (x / 2);
                    // U = -0.169*R - 0.331*G + 0.5*B + 128
                    u_plane[uv_idx] = 
                        static_cast<uint8_t>(-0.169f * r - 0.331f * g + 0.5f * b + 128);
                    // V = 0.5*R - 0.419*G - 0.081*B + 128
                    v_plane[uv_idx] = 
                        static_cast<uint8_t>(0.5f * r - 0.419f * g - 0.081f * b + 128);
                }
            }
        }
    }
    
    // Convert NV12 to YUV420
    void nv12_to_yuv420(const uint8_t* nv12, int width, int height,
                        uint8_t* yuv) {
        size_t y_size = width * height;
        uint8_t* y_plane = yuv;
        uint8_t* u_plane = yuv + y_size;
        uint8_t* v_plane = yuv + y_size + (width / 2) * (height / 2);
        
        // Copy Y plane
        memcpy(y_plane, nv12, y_size);
        
        // Deinterleave UV from NV12
        const uint8_t* uv_src = nv12 + y_size;
        int uv_width = width / 2;
        int uv_height = height / 2;
        for (int y = 0; y < uv_height; ++y) {
            for (int x = 0; x < uv_width; ++x) {
                int idx = y * width + x * 2;
                u_plane[y * uv_width + x] = uv_src[idx];
                v_plane[y * uv_width + x] = uv_src[idx + 1];
            }
        }
    }
};

VideoCompressor::VideoCompressor() : impl_(new Impl()) {}

VideoCompressor::~VideoCompressor() {
    delete impl_;
}

bool VideoCompressor::compress_frame(const uint8_t* input_data, size_t input_size,
                                     uint8_t* output_data, size_t* output_size,
                                     int width, int height, int format) {
    if (!input_data || !output_data || !output_size) {
        return false;
    }
    
    // Initialize encoder if not already done
    if (!impl_->initialized_) {
        EncodingParams params;
        params.width = width;
        params.height = height;
        params.fps = 30; // Default
        params.bitrate_kbps = 1000; // Default
        params.format = static_cast<VideoFormat>(format);
        params.use_ai_rate_control = impl_->ai_rate_control_enabled_;
        
        if (!initialize_encoder(params)) {
            return false;
        }
    }
    
    // Calculate YUV420 buffer size
    size_t yuv_size = width * height * 3 / 2; // Y + U + V
    
    // Convert input format to YUV420
    if (!impl_->ensure_yuv_buffer(yuv_size)) {
        return false;
    }
    
    uint8_t* yuv_data = impl_->yuv_buffer_;
    
    switch (static_cast<VideoFormat>(format)) {
        case VideoFormat::YUV420P:
            // Already in YUV420, just copy
            if (input_size < yuv_size) {
                return false;
            }
            memcpy(yuv_data, input_data, yuv_size);
            break;
            
        case VideoFormat::RGB24:
            impl_->rgb_to_yuv420(input_data, width, height, yuv_data);
            break;
            
        case VideoFormat::NV12:
            impl_->nv12_to_yuv420(input_data, width, height, yuv_data);
            break;
            
        case VideoFormat::RGBA:
            // Convert RGBA to RGB first, then to YUV
            {
                size_t rgb_size = width * height * 3;
                uint8_t* rgb_buffer = new uint8_t[rgb_size];
                for (int i = 0; i < width * height; ++i) {
                    rgb_buffer[i * 3] = input_data[i * 4];
                    rgb_buffer[i * 3 + 1] = input_data[i * 4 + 1];
                    rgb_buffer[i * 3 + 2] = input_data[i * 4 + 2];
                }
                impl_->rgb_to_yuv420(rgb_buffer, width, height, yuv_data);
                delete[] rgb_buffer;
            }
            break;
            
        default:
            return false;
    }
    
    // Prepare frame for encoding
    AV1Encoder::Frame frame;
    size_t y_size = width * height;
    size_t uv_size = (width / 2) * (height / 2);
    
    frame.y_plane = yuv_data;
    frame.u_plane = yuv_data + y_size;
    frame.v_plane = yuv_data + y_size + uv_size;
    frame.y_stride = width;
    frame.uv_stride = width / 2;
    frame.width = width;
    frame.height = height;
    frame.pts = 0;
    frame.force_keyframe = false;
    
    // Apply rate control if enabled
    if (impl_->ai_rate_control_enabled_) {
        uint32_t target_bitrate = impl_->params_.bitrate_kbps;
        uint32_t adjusted_bitrate = target_bitrate;
        
        FrameAnalysis analysis;
        if (impl_->rate_control_.compute_target_bitrate(
                yuv_data, width, height, target_bitrate, 
                &adjusted_bitrate, &analysis)) {
            // Adjust encoder bitrate (would need encoder API support)
            // For now, we just track it in stats
        }
    }
    
    // Send frame to encoder
    if (!impl_->encoder_.send_frame(&frame)) {
        return false;
    }
    
    // Receive encoded packet
    AV1Encoder::Packet packet;
    if (!impl_->encoder_.receive_packet(&packet)) {
        // May need more frames
        *output_size = 0;
        return true;
    }
    
    // Copy to output
    if (packet.size > *output_size) {
        return false; // Output buffer too small
    }
    
    memcpy(output_data, packet.data, packet.size);
    *output_size = packet.size;
    
    // Update compression stats
    impl_->stats_.original_size_bytes = input_size;
    impl_->stats_.compressed_size_bytes = packet.size;
    impl_->stats_.compression_ratio = 
        static_cast<float>(input_size) / static_cast<float>(packet.size);
    
    return true;
}

float VideoCompressor::get_compression_ratio() const {
    return impl_->stats_.compression_ratio;
}

void VideoCompressor::set_ai_rate_control(bool enabled) {
    impl_->ai_rate_control_enabled_ = enabled;
}

bool VideoCompressor::initialize_encoder(const EncodingParams& params) {
    impl_->params_ = params;
    
    // Initialize rate control if AI rate control is enabled
    if (params.use_ai_rate_control) {
        RateControl::Config rc_config;
        rc_config.target_bitrate_kbps = params.bitrate_kbps;
        rc_config.min_bitrate_kbps = params.bitrate_kbps / 2;
        rc_config.max_bitrate_kbps = params.bitrate_kbps * 2;
        rc_config.bitrate_variance = 0.2f;
        rc_config.enable_kip_mode = true;
        
        // Configure kip-mode
        rc_config.kip_config.target_bitrate_kbps = params.bitrate_kbps;
        rc_config.kip_config.skin_tone_boost = 1.5f;
        rc_config.kip_config.enable_skin_protection = params.use_african_skin_tone_optimization;
        rc_config.kip_config.enable_talking_head_detection = true;
        rc_config.kip_config.enable_clip_analysis = false; // Can be enabled if CLIP model available
        rc_config.kip_config.min_qp_skin = 20.0f;
        rc_config.kip_config.max_qp_background = 60.0f;
        
        if (!impl_->rate_control_.initialize(rc_config)) {
            return false;
        }
    }
    
    // Initialize encoder
    AV1Encoder::Config encoder_config;
    encoder_config.width = params.width;
    encoder_config.height = params.height;
    encoder_config.fps_num = params.fps;
    encoder_config.fps_den = 1;
    encoder_config.bitrate_kbps = params.bitrate_kbps;
    encoder_config.speed = 6; // Balanced speed/quality
    encoder_config.threads = 0; // Auto-detect
    encoder_config.use_kip_mode = params.use_ai_rate_control;
    
    if (params.use_ai_rate_control) {
        encoder_config.kip_config.target_bitrate_kbps = params.bitrate_kbps;
        encoder_config.kip_config.enable_skin_protection = params.use_african_skin_tone_optimization;
        encoder_config.kip_config.enable_talking_head_detection = true;
    }
    
    if (!impl_->encoder_.initialize(encoder_config)) {
        return false;
    }
    
    impl_->initialized_ = true;
    return true;
}

} // namespace video
} // namespace kipepeo
