#pragma once

#include "kipepeo/video/types.h"
#include "kipepeo/video/kip_mode_lowband.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <functional>

namespace kipepeo {
namespace video {

// Forward declarations for rav1e C API
// These will be defined when rav1e C header is available
struct RaContext;
struct RaConfig;
struct RaFrame;
struct RaPacket;

/**
 * AV1 Encoder - Wrapper for rav1e encoder
 * 
 * Integrates with kip-mode-lowband for AI-driven rate control
 * Optimized for real-time encoding on mobile devices
 */
class AV1Encoder {
public:
    AV1Encoder();
    ~AV1Encoder();

    /**
     * Encoder configuration
     */
    struct Config {
        int32_t width = 0;
        int32_t height = 0;
        int32_t fps_num = 30;           // FPS numerator
        int32_t fps_den = 1;            // FPS denominator
        int32_t bitrate_kbps = 1000;    // Target bitrate
        int32_t speed = 6;              // Speed preset (0-10, higher = faster)
        int32_t threads = 0;            // Number of threads (0 = auto-detect)
        bool use_kip_mode = true;      // Enable kip-mode-lowband
        RateControlConfig kip_config;   // kip-mode configuration
        bool low_latency = false;       // Enable low-latency mode
        bool use_hardware = false;      // Use hardware acceleration if available
    };

    /**
     * Frame to encode
     */
    struct Frame {
        const uint8_t* y_plane;         // Y plane data
        const uint8_t* u_plane;        // U plane data
        const uint8_t* v_plane;        // V plane data
        ptrdiff_t y_stride;             // Y plane stride
        ptrdiff_t uv_stride;            // UV plane stride
        int32_t width;                  // Frame width
        int32_t height;                 // Frame height
        int64_t pts;                    // Presentation timestamp
        bool force_keyframe = false;    // Force keyframe
    };

    /**
     * Encoded packet
     */
    struct Packet {
        const uint8_t* data;            // Encoded data
        size_t size;                    // Size in bytes
        int64_t pts;                    // Presentation timestamp
        bool is_keyframe;               // True if keyframe
        uint64_t frame_number;          // Frame sequence number
    };

    /**
     * Initialize encoder with configuration
     * 
     * @param config Encoder configuration
     * @return true on success
     */
    bool initialize(const Config& config);

    /**
     * Send frame for encoding
     * 
     * @param frame Frame to encode (nullptr to flush)
     * @return true on success
     */
    bool send_frame(const Frame* frame);

    /**
     * Receive encoded packet
     * 
     * @param packet Output encoded packet
     * @return true if packet available, false if more frames needed or error
     */
    bool receive_packet(Packet* packet);

    /**
     * Flush encoder (encode all buffered frames)
     * 
     * @param packet Output encoded packet
     * @return true if packet available, false when no more packets
     */
    bool flush(Packet* packet);

    /**
     * Get encoding statistics
     */
    struct Stats {
        uint64_t frames_encoded;
        uint64_t bytes_encoded;
        float average_bitrate_kbps;
        float average_psnr;
        float average_encoding_time_ms;
        KipModeLowband::Stats kip_stats;
    };

    Stats get_stats() const;

    /**
     * Check if hardware acceleration is available
     */
    static bool is_hardware_available();

    /**
     * Set per-macroblock QP callback (for kip-mode integration)
     * 
     * @param callback Function that returns QP for given macroblock position
     */
    void set_qp_callback(std::function<float(uint32_t mb_x, uint32_t mb_y, float base_qp)> callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace video
} // namespace kipepeo

