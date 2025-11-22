#pragma once

#include "kipepeo/video/types.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace kipepeo {
namespace video {

/**
 * AV1 Decoder - Wrapper for dav1d/rav1d decoder
 * 
 * Supports both rav1d (Rust port) and dav1d (C implementation)
 * Optimized for real-time decoding on mobile devices
 */
class AV1Decoder {
public:
    AV1Decoder();
    ~AV1Decoder();

    /**
     * Decoder configuration
     */
    struct Config {
        int32_t width = 0;              // Frame width (0 = auto-detect)
        int32_t height = 0;             // Frame height (0 = auto-detect)
        int32_t threads = 0;            // Number of threads (0 = auto-detect)
        int32_t max_frame_delay = 0;    // Max frame delay (0 = auto, 1 = low-latency)
        bool apply_grain = true;        // Apply film grain
        bool low_latency = false;       // Enable low-latency mode
        bool use_hardware = false;      // Use hardware acceleration if available
    };

    /**
     * Decoded frame information
     */
    struct DecodedFrame {
        uint8_t* data[3];           // Y, U, V plane pointers
        ptrdiff_t stride[2];        // Stride for Y and UV
        int32_t width;              // Frame width
        int32_t height;             // Frame height
        int64_t pts;                // Presentation timestamp
        bool is_keyframe;           // True if keyframe
    };

    /**
     * Initialize decoder with configuration
     * 
     * @param config Decoder configuration
     * @return true on success
     */
    bool initialize(const Config& config);

    /**
     * Feed compressed data to decoder
     * 
     * @param data Compressed AV1 data (OBU format)
     * @param size Size of data in bytes
     * @param pts Presentation timestamp (optional)
     * @return true on success, false on error
     */
    bool send_data(const uint8_t* data, size_t size, int64_t pts = 0);

    /**
     * Retrieve decoded frame
     * 
     * @param frame Output decoded frame (caller must free data if allocated)
     * @return true if frame available, false if more data needed or error
     */
    bool get_frame(DecodedFrame* frame);

    /**
     * Flush decoder (drain all buffered frames)
     * 
     * @param frame Output decoded frame
     * @return true if frame available, false when no more frames
     */
    bool flush(DecodedFrame* frame);

    /**
     * Reset decoder state (for seeking)
     */
    void reset();

    /**
     * Get decoder statistics
     */
    struct Stats {
        uint64_t frames_decoded;
        uint64_t bytes_processed;
        float average_decode_time_ms;
        uint32_t dropped_frames;
    };

    Stats get_stats() const;

    /**
     * Check if hardware acceleration is available
     */
    static bool is_hardware_available();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace video
} // namespace kipepeo

