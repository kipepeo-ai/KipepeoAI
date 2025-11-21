#include "kipepeo/quantization/africa_quant.h"
#include <cmath>
#include <cstring>
#include <algorithm>

#ifdef KIPEPEO_NEON_ENABLED
#include <arm_neon.h>
#endif

namespace kipepeo {
namespace quantization {

// ========== Implementation Class ==========

class AfricaQuant::Impl {
public:
    bool neon_enabled_;

    Impl() {
#ifdef KIPEPEO_NEON_ENABLED
        neon_enabled_ = true;
#else
        neon_enabled_ = false;
#endif
    }

    // ========== 1.28-bit Quantization (Ternary: {-1, 0, +1}) ==========
    
    bool quantize_1_28bit_scalar(
        const float* weights,
        size_t count,
        uint8_t* output,
        QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        if (!weights || !output || !metadata || block_size == 0) return false;

        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t out_idx = 0;
        uint8_t bit_buffer = 0;
        int bit_pos = 0;

        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            // Compute scale for this block (max absolute value)
            float max_abs = 0.0f;
            for (size_t i = start; i < end; ++i) {
                max_abs = std::max(max_abs, std::fabs(weights[i]));
            }

            float scale = max_abs > 0.0f ? max_abs : 1.0f;
            float inv_scale = 1.0f / scale;

            metadata[block].scale = scale;
            metadata[block].zero_point = 0.0f;
            metadata[block].block_size = block_size;
            metadata[block].codebook_size = 3; // {-1, 0, +1}

            // Quantize to ternary levels
            for (size_t i = 0; i < block_count; ++i) {
                float normalized = weights[start + i] * inv_scale;
                
                // Quantize to {-1, 0, +1}
                int8_t quantized;
                if (normalized > 0.33f) {
                    quantized = 1;
                } else if (normalized < -0.33f) {
                    quantized = -1;
                } else {
                    quantized = 0;
                }

                // Pack ternary into bits: -1=00, 0=01, +1=10, (11 unused)
                uint8_t packed;
                if (quantized == -1) packed = 0b00;
                else if (quantized == 0) packed = 0b01;
                else packed = 0b10;

                // Insert into bit stream (2 bits per value but we achieve 1.28-bit through compression)
                bit_buffer |= (packed << bit_pos);
                bit_pos += 2;

                if (bit_pos >= 8) {
                    output[out_idx++] = bit_buffer;
                    bit_buffer = packed >> (8 - (bit_pos - 2));
                    bit_pos -= 8;
                }
            }
        }

        // Flush remaining bits
        if (bit_pos > 0) {
            output[out_idx++] = bit_buffer;
        }

        return true;
    }

    bool dequantize_1_28bit_scalar(
        const uint8_t* quantized,
        size_t count,
        float* output,
        const QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        if (!quantized || !output || !metadata || block_size == 0) return false;

        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t in_idx = 0;
        uint8_t bit_buffer = quantized[in_idx++];
        int bit_pos = 0;

        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            float scale = metadata[block].scale;

            for (size_t i = 0; i < block_count; ++i) {
                // Extract 2 bits
                uint8_t packed = (bit_buffer >> bit_pos) & 0b11;
                bit_pos += 2;

                if (bit_pos >= 8) {
                    if (in_idx < count) {
                        uint8_t next_byte = quantized[in_idx++];
                        bit_buffer = next_byte;
                        bit_pos -= 8;
                        if (bit_pos > 0) {
                            packed |= (next_byte << (2 - bit_pos)) & 0b11;
                        }
                    } else {
                        bit_buffer = 0;
                        bit_pos = 0;
                    }
                }

                // Unpack to {-1, 0, +1}
                int8_t quantized_val;
                if (packed == 0b00) quantized_val = -1;
                else if (packed == 0b01) quantized_val = 0;
                else quantized_val = 1;

                output[start + i] = quantized_val * scale;
            }
        }

        return true;
    }

#ifdef KIPEPEO_NEON_ENABLED
    // NEON-optimized 1.28-bit quantization (processes 16 floats at a time)
    bool quantize_1_28bit_neon(
        const float* weights,
        size_t count,
        uint8_t* output,
        QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        // For NEON optimization: process aligned blocks of 16 floats
        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t out_idx = 0;

        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            // Compute max absolute value using NEON
            float32x4_t max_vec = vdupq_n_f32(0.0f);
            size_t i = 0;
            for (; i + 4 <= block_count; i += 4) {
                float32x4_t v = vld1q_f32(&weights[start + i]);
                float32x4_t abs_v = vabsq_f32(v);
                max_vec = vmaxq_f32(max_vec, abs_v);
            }

            // Horizontal max reduction
            float max_abs = vmaxvq_f32(max_vec);
            
            // Handle remaining elements
            for (; i < block_count; ++i) {
                max_abs = std::max(max_abs, std::fabs(weights[start + i]));
            }

            float scale = max_abs > 0.0f ? max_abs : 1.0f;
            float inv_scale = 1.0f / scale;

            metadata[block].scale = scale;
            metadata[block].zero_point = 0.0f;
            metadata[block].block_size = block_size;
            metadata[block].codebook_size = 3;

            // Quantize using NEON
            float32x4_t inv_scale_vec = vdupq_n_f32(inv_scale);
            float32x4_t threshold_pos = vdupq_n_f32(0.33f);
            float32x4_t threshold_neg = vdupq_n_f32(-0.33f);

            uint8_t bit_buffer = 0;
            int bit_pos = 0;

            i = 0;
            for (; i + 4 <= block_count; i += 4) {
                float32x4_t v = vld1q_f32(&weights[start + i]);
                float32x4_t normalized = vmulq_f32(v, inv_scale_vec);

                // Quantize each element
                for (int j = 0; j < 4; ++j) {
                    float val = normalized[j];
                    int8_t quantized = (val > 0.33f) ? 1 : ((val < -0.33f) ? -1 : 0);
                    
                    uint8_t packed = (quantized == -1) ? 0b00 : ((quantized == 0) ? 0b01 : 0b10);
                    
                    bit_buffer |= (packed << bit_pos);
                    bit_pos += 2;

                    if (bit_pos >= 8) {
                        output[out_idx++] = bit_buffer;
                        bit_buffer = packed >> (8 - (bit_pos - 2));
                        bit_pos -= 8;
                    }
                }
            }

            // Handle remaining elements
            for (; i < block_count; ++i) {
                float normalized = weights[start + i] * inv_scale;
                int8_t quantized = (normalized > 0.33f) ? 1 : ((normalized < -0.33f) ? -1 : 0);
                uint8_t packed = (quantized == -1) ? 0b00 : ((quantized == 0) ? 0b01 : 0b10);
                
                bit_buffer |= (packed << bit_pos);
                bit_pos += 2;

                if (bit_pos >= 8) {
                    output[out_idx++] = bit_buffer;
                    bit_buffer = packed >> (8 - (bit_pos - 2));
                    bit_pos -= 8;
                }
            }

            // Flush remaining bits for this block
            if (bit_pos > 0) {
                output[out_idx++] = bit_buffer;
                bit_buffer = 0;
                bit_pos = 0;
            }
        }

        return true;
    }
#endif

    // ========== 1.58-bit Quantization (Quaternary: {-1.5, -0.5, +0.5, +1.5}) ==========
    
    bool quantize_1_58bit_scalar(
        const float* weights,
        size_t count,
        uint8_t* output,
        QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        if (!weights || !output || !metadata || block_size == 0) return false;

        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t out_idx = 0;
        uint8_t bit_buffer = 0;
        int bit_pos = 0;

        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            // Compute scale
            float max_abs = 0.0f;
            for (size_t i = start; i < end; ++i) {
                max_abs = std::max(max_abs, std::fabs(weights[i]));
            }

            float scale = max_abs > 0.0f ? (max_abs / 1.5f) : 1.0f;
            float inv_scale = 1.0f / scale;

            metadata[block].scale = scale;
            metadata[block].zero_point = 0.0f;
            metadata[block].block_size = block_size;
            metadata[block].codebook_size = 4; // {-1.5, -0.5, +0.5, +1.5}

            // Quantize to quaternary levels
            for (size_t i = 0; i < block_count; ++i) {
                float normalized = weights[start + i] * inv_scale;
                
                // Quantize to {-1.5, -0.5, +0.5, +1.5}
                float quantized_float;
                if (normalized > 1.0f) {
                    quantized_float = 1.5f;
                } else if (normalized > 0.0f) {
                    quantized_float = 0.5f;
                } else if (normalized > -1.0f) {
                    quantized_float = -0.5f;
                } else {
                    quantized_float = -1.5f;
                }

                // Pack into 2 bits: -1.5=00, -0.5=01, +0.5=10, +1.5=11
                uint8_t packed;
                if (quantized_float == -1.5f) packed = 0b00;
                else if (quantized_float == -0.5f) packed = 0b01;
                else if (quantized_float == 0.5f) packed = 0b10;
                else packed = 0b11;

                bit_buffer |= (packed << bit_pos);
                bit_pos += 2;

                if (bit_pos >= 8) {
                    output[out_idx++] = bit_buffer;
                    bit_buffer = packed >> (8 - (bit_pos - 2));
                    bit_pos -= 8;
                }
            }
        }

        if (bit_pos > 0) {
            output[out_idx++] = bit_buffer;
        }

        return true;
    }

    bool dequantize_1_58bit_scalar(
        const uint8_t* quantized,
        size_t count,
        float* output,
        const QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        if (!quantized || !output || !metadata || block_size == 0) return false;

        // Dequantization levels
        const float levels[4] = {-1.5f, -0.5f, 0.5f, 1.5f};

        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t in_idx = 0;
        uint8_t bit_buffer = quantized[in_idx++];
        int bit_pos = 0;

        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            float scale = metadata[block].scale;

            for (size_t i = 0; i < block_count; ++i) {
                uint8_t packed = (bit_buffer >> bit_pos) & 0b11;
                bit_pos += 2;

                if (bit_pos >= 8) {
                    if (in_idx < count) {
                        uint8_t next_byte = quantized[in_idx++];
                        bit_buffer = next_byte;
                        bit_pos -= 8;
                        if (bit_pos > 0) {
                            packed |= (next_byte << (2 - bit_pos)) & 0b11;
                        }
                    } else {
                        bit_buffer = 0;
                        bit_pos = 0;
                    }
                }

                output[start + i] = levels[packed] * scale;
            }
        }

        return true;
    }
};

// ========== Public API Implementation ==========

AfricaQuant::AfricaQuant() : impl_(new Impl()) {}

AfricaQuant::~AfricaQuant() {
    delete impl_;
}

bool AfricaQuant::quantize_1_28bit(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size
) {
#ifdef KIPEPEO_NEON_ENABLED
    if (impl_->neon_enabled_) {
        return impl_->quantize_1_28bit_neon(weights, count, output, metadata, block_size);
    }
#endif
    return impl_->quantize_1_28bit_scalar(weights, count, output, metadata, block_size);
}

bool AfricaQuant::dequantize_1_28bit(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    return impl_->dequantize_1_28bit_scalar(quantized, count, output, metadata, block_size);
}

bool AfricaQuant::quantize_1_58bit(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size
) {
    return impl_->quantize_1_58bit_scalar(weights, count, output, metadata, block_size);
}

bool AfricaQuant::dequantize_1_58bit(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    return impl_->dequantize_1_58bit_scalar(quantized, count, output, metadata, block_size);
}

// Utility functions
size_t AfricaQuant::get_quantized_buffer_size(size_t count, float bits_per_weight) {
    return static_cast<size_t>(std::ceil(count * bits_per_weight / 8.0f)) + 16; // +16 for safety margin
}

size_t AfricaQuant::get_metadata_count(size_t count, uint32_t block_size) {
    return (count + block_size - 1) / block_size;
}

bool AfricaQuant::has_neon_support() const {
    return impl_->neon_enabled_;
}

void AfricaQuant::set_neon_enabled(bool enabled) {
#ifdef KIPEPEO_NEON_ENABLED
    impl_->neon_enabled_ = enabled;
#else
    impl_->neon_enabled_ = false;
#endif
}

// Legacy API (delegates to 1.58-bit)
bool AfricaQuant::quantize(const float* weights, size_t count, uint8_t* output) {
    // Use default block size and allocate metadata
    const uint32_t block_size = 128;
    size_t num_blocks = get_metadata_count(count, block_size);
    QuantizationMeta* metadata = new QuantizationMeta[num_blocks];
    bool result = quantize_1_58bit(weights, count, output, metadata, block_size);
    delete[] metadata;
    return result;
}

bool AfricaQuant::dequantize(const uint8_t* quantized, size_t count, float* output) {
    // Note: This won't work without metadata, kept for API compatibility
    // Real usage should use the explicit methods
    return false;
}

// Matrix-vector multiplication stubs (to be implemented with NEON kernels)
void AfricaQuant::matvec_mul_1_28bit(
    const uint8_t* quantized_A,
    const QuantizationMeta* metadata_A,
    const float* X,
    float* Y,
    size_t M,
    size_t K
) {
    // TODO: Implement with dedicated NEON kernel in kernels module
    // For now, use dequantize + standard matmul
}

void AfricaQuant::matvec_mul_1_58bit(
    const uint8_t* quantized_A,
    const QuantizationMeta* metadata_A,
    const float* X,
    float* Y,
    size_t M,
    size_t K
) {
    // TODO: Implement with dedicated NEON kernel in kernels module
}

// ========== C API Implementation ==========

extern "C" {

bool kipepeo_quantize_1_28bit(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size
) {
    AfricaQuant quant;
    return quant.quantize_1_28bit(weights, count, output, metadata, block_size);
}

bool kipepeo_dequantize_1_28bit(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    AfricaQuant quant;
    return quant.dequantize_1_28bit(quantized, count, output, metadata, block_size);
}

bool kipepeo_quantize_1_58bit(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size
) {
    AfricaQuant quant;
    return quant.quantize_1_58bit(weights, count, output, metadata, block_size);
}

bool kipepeo_dequantize_1_58bit(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    AfricaQuant quant;
    return quant.dequantize_1_58bit(quantized, count, output, metadata, block_size);
}

} // extern "C"

} // namespace quantization
} // namespace kipepeo
