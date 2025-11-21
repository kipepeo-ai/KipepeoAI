#pragma once

#include <cstdint>

namespace kipepeo {
namespace video {

// Video formats
enum class VideoFormat {
    YUV420P,
    NV12,
    RGB24,
    RGBA
};

// Encoding parameters
struct EncodingParams {
    int32_t width = 0;
    int32_t height = 0;
    int32_t fps = 30;
    int32_t bitrate_kbps = 1000;
    VideoFormat format = VideoFormat::YUV420P;
    bool use_african_skin_tone_optimization = true;
    bool use_ai_rate_control = true;
};

// Compression statistics
struct CompressionStats {
    float compression_ratio = 1.0f;
    size_t original_size_bytes = 0;
    size_t compressed_size_bytes = 0;
    float psnr = 0.0f;
    float ssim = 0.0f;
};

} // namespace video
} // namespace kipepeo

