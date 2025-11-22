#include "kipepeo/video/av1_decoder.h"
#include <cstring>
#include <algorithm>
#include <chrono>

// Include dav1d/rav1d headers based on availability
#ifdef KIPEPEO_USE_RAV1D
// rav1d provides dav1d-compatible C API
#include "dav1d/dav1d.h"
#include "dav1d/picture.h"
#include "dav1d/data.h"
#elif defined(KIPEPEO_USE_DAV1D)
#include "dav1d/dav1d.h"
#include "dav1d/picture.h"
#include "dav1d/data.h"
#else
// Fallback: define minimal structures if neither is available
// This allows compilation but runtime will fail gracefully
struct Dav1dContext {};
struct Dav1dSettings {};
struct Dav1dPicture {};
struct Dav1dData {};
#endif

#ifdef KIPEPEO_NEON_ENABLED
#include <arm_neon.h>
#endif

namespace kipepeo {
namespace video {

class AV1Decoder::Impl {
public:
#if defined(KIPEPEO_USE_RAV1D) || defined(KIPEPEO_USE_DAV1D)
    Dav1dContext* ctx_ = nullptr;
#else
    void* ctx_ = nullptr;
#endif
    Config config_;
    Stats stats_;
    bool initialized_ = false;
    
    // Frame buffer pool for real-time performance
    struct FrameBuffer {
        uint8_t* data[3];
        ptrdiff_t stride[2];
        size_t size[3];
        bool in_use;
    };
    std::vector<FrameBuffer> frame_pool_;
    static constexpr size_t POOL_SIZE = 4; // Keep 4 frames in pool
    
    // Timing for performance stats
    std::chrono::high_resolution_clock::time_point decode_start_;
    
    Impl() {
        memset(&stats_, 0, sizeof(stats_));
    }
    
    ~Impl() {
        cleanup();
    }
    
    void cleanup() {
#if defined(KIPEPEO_USE_RAV1D) || defined(KIPEPEO_USE_DAV1D)
        if (ctx_) {
            dav1d_close(&ctx_);
            ctx_ = nullptr;
        }
#endif
        
        // Free frame pool
        for (auto& buf : frame_pool_) {
            for (int i = 0; i < 3; ++i) {
                if (buf.data[i]) {
                    delete[] buf.data[i];
                    buf.data[i] = nullptr;
                }
            }
        }
        frame_pool_.clear();
    }
    
    FrameBuffer* allocate_frame_buffer(int width, int height) {
        // Try to find unused buffer in pool
        for (auto& buf : frame_pool_) {
            if (!buf.in_use && buf.size[0] >= static_cast<size_t>(width * height)) {
                buf.in_use = true;
                return &buf;
            }
        }
        
        // Allocate new buffer if pool not full
        if (frame_pool_.size() < POOL_SIZE) {
            FrameBuffer buf;
            memset(&buf, 0, sizeof(buf));
            
            // Allocate Y plane (64-byte aligned for SIMD)
            size_t y_size = (width * height + 63) & ~63;
            buf.data[0] = new (std::align_val_t(64)) uint8_t[y_size];
            buf.size[0] = y_size;
            buf.stride[0] = width;
            
            // Allocate UV planes (subsampled)
            size_t uv_size = ((width / 2) * (height / 2) + 63) & ~63;
            buf.data[1] = new (std::align_val_t(64)) uint8_t[uv_size];
            buf.data[2] = new (std::align_val_t(64)) uint8_t[uv_size];
            buf.size[1] = uv_size;
            buf.size[2] = uv_size;
            buf.stride[1] = width / 2;
            
            buf.in_use = true;
            frame_pool_.push_back(buf);
            return &frame_pool_.back();
        }
        
        return nullptr; // Pool exhausted
    }
    
    void release_frame_buffer(FrameBuffer* buf) {
        if (buf) {
            buf->in_use = false;
        }
    }
    
    void copy_yuv_frame(const Dav1dPicture* picture, DecodedFrame* frame);
};

AV1Decoder::AV1Decoder() : impl_(std::make_unique<Impl>()) {}

AV1Decoder::~AV1Decoder() = default;

bool AV1Decoder::initialize(const Config& config) {
    if (impl_->initialized_) {
        impl_->cleanup();
    }
    
    impl_->config_ = config;
    
#if defined(KIPEPEO_USE_RAV1D) || defined(KIPEPEO_USE_DAV1D)
    Dav1dSettings settings;
    dav1d_default_settings(&settings);
    
    // Configure threads
    if (config.threads > 0) {
        settings.n_threads = config.threads;
    } else {
        // Auto-detect: use number of CPU cores
        settings.n_threads = 0; // 0 = auto-detect
    }
    
    // Configure frame delay for low-latency
    if (config.low_latency || config.max_frame_delay == 1) {
        settings.max_frame_delay = 1;
    } else if (config.max_frame_delay > 0) {
        settings.max_frame_delay = config.max_frame_delay;
    } else {
        settings.max_frame_delay = 0; // Auto
    }
    
    // Film grain
    settings.apply_grain = config.apply_grain ? 1 : 0;
    
    // Low-latency optimizations
    if (config.low_latency) {
        settings.inloop_filters = DAV1D_INLOOPFILTER_DEBLOCK; // Skip expensive filters
    } else {
        settings.inloop_filters = DAV1D_INLOOPFILTER_ALL;
    }
    
    // Open decoder
    int ret = dav1d_open(&impl_->ctx_, &settings);
    if (ret < 0) {
        return false;
    }
    
    impl_->initialized_ = true;
    return true;
#else
    // No decoder available - return false but don't crash
    return false;
#endif
}

bool AV1Decoder::send_data(const uint8_t* data, size_t size, int64_t pts) {
    if (!impl_->initialized_ || !data || size == 0) {
        return false;
    }
    
#if defined(KIPEPEO_USE_RAV1D) || defined(KIPEPEO_USE_DAV1D)
    Dav1dData dav1d_data;
    memset(&dav1d_data, 0, sizeof(dav1d_data));
    
    // Wrap data (dav1d will take ownership)
    uint8_t* data_copy = dav1d_data_create(&dav1d_data, size);
    if (!data_copy) {
        return false;
    }
    
    memcpy(data_copy, data, size);
    dav1d_data.sz = size;
    
    // Set PTS in metadata if supported
    // (dav1d doesn't directly support PTS, but we can store it for later)
    
    int ret = dav1d_send_data(impl_->ctx_, &dav1d_data);
    
    if (ret < 0 && ret != DAV1D_ERR(EAGAIN)) {
        dav1d_data_unref(&dav1d_data);
        return false;
    }
    
    // Update stats
    impl_->stats_.bytes_processed += size;
    
    return true;
#else
    return false;
#endif
}

bool AV1Decoder::get_frame(DecodedFrame* frame) {
    if (!impl_->initialized_ || !frame) {
        return false;
    }
    
#if defined(KIPEPEO_USE_RAV1D) || defined(KIPEPEO_USE_DAV1D)
    auto start_time = std::chrono::high_resolution_clock::now();
    
    Dav1dPicture picture;
    memset(&picture, 0, sizeof(picture));
    
    int ret = dav1d_get_picture(impl_->ctx_, &picture);
    
    if (ret < 0) {
        if (ret == DAV1D_ERR(EAGAIN)) {
            return false; // Need more data
        }
        return false; // Error
    }
    
    // Copy frame data
    frame->width = picture.p.w;
    frame->height = picture.p.h;
    frame->pts = 0; // PTS not directly available from dav1d
    
    // Check if keyframe (simplified - check frame header)
    frame->is_keyframe = false;
    if (picture.frame_hdr) {
        // DAV1D_FRAME_TYPE_KEY = 0, DAV1D_FRAME_TYPE_INTRA = 2
        enum Dav1dFrameType frame_type = picture.frame_hdr->frame_type;
        frame->is_keyframe = (frame_type == DAV1D_FRAME_TYPE_KEY || 
                              frame_type == DAV1D_FRAME_TYPE_INTRA);
    }
    
    // Allocate frame buffer from pool
    Impl::FrameBuffer* buf = impl_->allocate_frame_buffer(frame->width, frame->height);
    if (!buf) {
        // Pool exhausted - allocate temporary buffer
        frame->data[0] = new uint8_t[frame->width * frame->height];
        frame->data[1] = new uint8_t[(frame->width / 2) * (frame->height / 2)];
        frame->data[2] = new uint8_t[(frame->width / 2) * (frame->height / 2)];
        frame->stride[0] = frame->width;
        frame->stride[1] = frame->width / 2;
        
    // Copy data
    impl_->copy_yuv_frame(&picture, frame);
        
        dav1d_picture_unref(&picture);
        return true;
    }
    
    // Use pooled buffer
    frame->data[0] = buf->data[0];
    frame->data[1] = buf->data[1];
    frame->data[2] = buf->data[2];
    frame->stride[0] = buf->stride[0];
    frame->stride[1] = buf->stride[1];
    
    // Copy YUV data
    impl_->copy_yuv_frame(&picture, frame);
    
    dav1d_picture_unref(&picture);
    
    // Update stats
    auto end_time = std::chrono::high_resolution_clock::now();
    auto decode_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time).count() / 1000.0f;
    
    impl_->stats_.frames_decoded++;
    impl_->stats_.average_decode_time_ms = 
        (impl_->stats_.average_decode_time_ms * (impl_->stats_.frames_decoded - 1) + 
         decode_time) / impl_->stats_.frames_decoded;
    
    return true;
#else
    return false;
#endif
}

bool AV1Decoder::flush(DecodedFrame* frame) {
    // Keep calling get_frame until no more frames
    return get_frame(frame);
}

void AV1Decoder::reset() {
    if (!impl_->initialized_) {
        return;
    }
    
#if defined(KIPEPEO_USE_RAV1D) || defined(KIPEPEO_USE_DAV1D)
    if (impl_->ctx_) {
        dav1d_flush(impl_->ctx_);
    }
#endif
    
    // Reset frame pool
    for (auto& buf : impl_->frame_pool_) {
        impl_->release_frame_buffer(&buf);
    }
}

AV1Decoder::Stats AV1Decoder::get_stats() const {
    return impl_->stats_;
}

bool AV1Decoder::is_hardware_available() {
    // Check for hardware acceleration
    // On Android: Check MediaCodec AV1 support
    // On iOS: Check VideoToolbox AV1 support
    // For now, return false (software only)
    return false;
}

// Helper function to copy YUV frame data
void AV1Decoder::Impl::copy_yuv_frame(const Dav1dPicture* picture, DecodedFrame* frame) {
    if (!picture || !frame) return;
    
#if defined(KIPEPEO_USE_RAV1D) || defined(KIPEPEO_USE_DAV1D)
    const uint8_t* src_y = static_cast<const uint8_t*>(picture->data[0]);
    const uint8_t* src_u = static_cast<const uint8_t*>(picture->data[1]);
    const uint8_t* src_v = static_cast<const uint8_t*>(picture->data[2]);
    
    ptrdiff_t src_stride_y = picture->stride[0];
    ptrdiff_t src_stride_uv = picture->stride[1];
    
    int width = frame->width;
    int height = frame->height;
    
    // Copy Y plane
#ifdef KIPEPEO_NEON_ENABLED
    // NEON-optimized copy
    int y = 0;
    for (; y + 16 <= height; y += 16) {
        int x = 0;
        for (; x + 16 <= width; x += 16) {
            uint8x16_t y_vec = vld1q_u8(&src_y[y * src_stride_y + x]);
            vst1q_u8(&frame->data[0][y * frame->stride[0] + x], y_vec);
        }
        // Handle remainder
        for (; x < width; ++x) {
            frame->data[0][y * frame->stride[0] + x] = src_y[y * src_stride_y + x];
        }
    }
    // Handle remainder rows
    for (; y < height; ++y) {
        memcpy(&frame->data[0][y * frame->stride[0]], 
               &src_y[y * src_stride_y], width);
    }
#else
    // Scalar copy
    for (int y = 0; y < height; ++y) {
        memcpy(&frame->data[0][y * frame->stride[0]], 
               &src_y[y * src_stride_y], width);
    }
#endif
    
    // Copy U and V planes (subsampled)
    int uv_width = width / 2;
    int uv_height = height / 2;
    
#ifdef KIPEPEO_NEON_ENABLED
    // NEON-optimized UV copy
    for (int y = 0; y < uv_height; ++y) {
        int x = 0;
        for (; x + 16 <= uv_width; x += 16) {
            uint8x16_t u_vec = vld1q_u8(&src_u[y * src_stride_uv + x]);
            uint8x16_t v_vec = vld1q_u8(&src_v[y * src_stride_uv + x]);
            vst1q_u8(&frame->data[1][y * frame->stride[1] + x], u_vec);
            vst1q_u8(&frame->data[2][y * frame->stride[1] + x], v_vec);
        }
        // Handle remainder
        for (; x < uv_width; ++x) {
            frame->data[1][y * frame->stride[1] + x] = src_u[y * src_stride_uv + x];
            frame->data[2][y * frame->stride[1] + x] = src_v[y * src_stride_uv + x];
        }
    }
#else
    // Scalar copy
    for (int y = 0; y < uv_height; ++y) {
        memcpy(&frame->data[1][y * frame->stride[1]], 
               &src_u[y * src_stride_uv], uv_width);
        memcpy(&frame->data[2][y * frame->stride[1]], 
               &src_v[y * src_stride_uv], uv_width);
    }
#endif
#endif
}

} // namespace video
} // namespace kipepeo
