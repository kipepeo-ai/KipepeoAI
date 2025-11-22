#pragma once

// Minimal rav1e C API declarations
// Full API should be generated via cbindgen from third_party/rav1e

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// Forward declarations
typedef struct RaContext RaContext;
typedef struct RaConfig RaConfig;
typedef struct RaFrame RaFrame;
typedef struct RaPacket RaPacket;

// Rational number
typedef struct {
    int num;
    int den;
} RaRational;

// Enums (simplified)
typedef enum {
    RA_ENCODER_STATUS_SUCCESS = 0,
    RA_ENCODER_STATUS_NEED_MORE_DATA = 1,
    RA_ENCODER_STATUS_ENOUGH_DATA = 2,
    RA_ENCODER_STATUS_LIMIT_REACHED = 3,
    RA_ENCODER_STATUS_ENCODED = 4,
    RA_ENCODER_STATUS_FAILURE = -1,
    RA_ENCODER_STATUS_NOT_READY = -2
} RaEncoderStatus;

typedef enum {
    RA_CHROMA_SAMPLING_420 = 0,
    RA_CHROMA_SAMPLING_422 = 1,
    RA_CHROMA_SAMPLING_444 = 2
} RaChromaSampling;

typedef enum {
    RA_PIXEL_RANGE_LIMITED = 0,
    RA_PIXEL_RANGE_FULL = 1
} RaPixelRange;

typedef enum {
    RA_FRAME_TYPE_OVERRIDE_NO = 0,
    RA_FRAME_TYPE_OVERRIDE_KEY = 1,
    RA_FRAME_TYPE_OVERRIDE_INTRA = 2
} RaFrameTypeOverride;

// Config functions
RaConfig* rav1e_config_default(void);
void rav1e_config_unref(RaConfig* cfg);
int rav1e_config_parse(RaConfig* cfg, const char* key, const char* value);
int rav1e_config_parse_int(RaConfig* cfg, const char* key, int value);
// Config setters (use parse instead for dimensions)
// int rav1e_config_set_dimensions(RaConfig* cfg, int width, int height);
int rav1e_config_set_time_base(RaConfig* cfg, RaRational time_base);
int rav1e_config_set_pixel_format(RaConfig* cfg, uint8_t bit_depth, 
                                   RaChromaSampling subsampling,
                                   int chroma_pos, RaPixelRange pixel_range);

// Context functions
RaContext* rav1e_context_new(const RaConfig* cfg);
void rav1e_context_unref(RaContext* ctx);

// Frame functions
RaFrame* rav1e_frame_new(const RaContext* ctx);
void rav1e_frame_unref(RaFrame* frame);
void rav1e_frame_fill_plane(RaFrame* frame, int plane, const uint8_t* data,
                            size_t data_len, ptrdiff_t stride, int bytewidth);
int rav1e_frame_set_type(RaFrame* frame, RaFrameTypeOverride frame_type);

// Encoding functions
RaEncoderStatus rav1e_send_frame(RaContext* ctx, RaFrame* frame);
RaEncoderStatus rav1e_receive_packet(RaContext* ctx, RaPacket** packet);

// Packet functions
void rav1e_packet_unref(RaPacket* packet);
const uint8_t* rav1e_packet_data(const RaPacket* packet);
size_t rav1e_packet_len(const RaPacket* packet);
uint64_t rav1e_packet_input_frameno(const RaPacket* packet);
int rav1e_packet_frame_type(const RaPacket* packet);

// Version
const char* rav1e_version_short(void);

#ifdef __cplusplus
}
#endif

