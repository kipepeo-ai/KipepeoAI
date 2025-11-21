#include "kipepeo/kernels/neon/quantized_gemm.h"
#include <cstring>
#include <algorithm>

#ifdef KIPEPEO_NEON_ENABLED
#include <arm_neon.h>
#endif

namespace kipepeo {
namespace kernels {
namespace neon {

// ========== Ternary (1.28-bit) Quantized GEMV ==========

void gemv_ternary_1_28bit(
    size_t M,
    size_t K,
    float alpha,
    const uint8_t* A_quantized,
    const float* A_scales,
    const float* X,
    float beta,
    float* Y,
    size_t block_size
) {
#ifdef KIPEPEO_NEON_ENABLED
    // NEON-optimized ternary matrix-vector multiplication
    // Ternary values: {-1, 0, +1} packed as 2 bits each
    // Encoding: -1=00, 0=01, +1=10
    
    size_t num_blocks_per_row = (K + block_size - 1) / block_size;

    // Scale Y by beta if beta != 1.0
    if (beta == 0.0f) {
        memset(Y, 0, M * sizeof(float));
    } else if (beta != 1.0f) {
        float32x4_t beta_vec = vdupq_n_f32(beta);
        size_t i = 0;
        for (; i + 4 <= M; i += 4) {
            float32x4_t y_vec = vld1q_f32(&Y[i]);
            y_vec = vmulq_f32(y_vec, beta_vec);
            vst1q_f32(&Y[i], y_vec);
        }
        for (; i < M; ++i) {
            Y[i] *= beta;
        }
    }

    // Process each row
    for (size_t row = 0; row < M; ++row) {
        float32x4_t acc = vdupq_n_f32(0.0f);
        
        size_t bit_offset = row * ((K * 2 + 7) / 8) * 8; // Bit offset for this row
        size_t byte_pos = bit_offset / 8;
        int bit_pos = bit_offset % 8;

        // Process by blocks
        for (size_t block_idx = 0; block_idx < num_blocks_per_row; ++block_idx) {
            size_t k_start = block_idx * block_size;
            size_t k_end = std::min(k_start + block_size, K);
            float scale = A_scales[row * num_blocks_per_row + block_idx];

            float32x4_t scale_vec = vdupq_n_f32(scale * alpha);

            // Process 4 elements at a time
            size_t k = k_start;
            for (; k + 4 <= k_end; k += 4) {
                // Load 4 X values
                float32x4_t x_vec = vld1q_f32(&X[k]);

                // Unpack 4 ternary values (8 bits total)
                int8_t ternary_vals[4];
                for (int i = 0; i < 4; ++i) {
                    // Extract 2 bits
                    uint8_t byte_val = A_quantized[byte_pos];
                    uint8_t packed = (byte_val >> bit_pos) & 0b11;
                    
                    bit_pos += 2;
                    if (bit_pos >= 8) {
                        byte_pos++;
                        bit_pos = 0;
                    }

                    // Decode: 00=-1, 01=0, 10=+1
                    if (packed == 0b00) ternary_vals[i] = -1;
                    else if (packed == 0b01) ternary_vals[i] = 0;
                    else ternary_vals[i] = 1;
                }

                // Convert to float and multiply with X
                float32x4_t q_vec = vcvtq_f32_s32(vmovl_s16(vget_low_s16(vmovl_s8(vld1_s8(ternary_vals)))));
                q_vec = vmulq_f32(q_vec, scale_vec);
                acc = vfmaq_f32(acc, q_vec, x_vec);
            }

            // Handle remaining elements
            for (; k < k_end; ++k) {
                uint8_t byte_val = A_quantized[byte_pos];
                uint8_t packed = (byte_val >> bit_pos) & 0b11;
                
                bit_pos += 2;
                if (bit_pos >= 8) {
                    byte_pos++;
                    bit_pos = 0;
                }

                int8_t ternary_val = (packed == 0b00) ? -1 : ((packed == 0b01) ? 0 : 1);
                
                Y[row] += ternary_val * scale * alpha * X[k];
            }
        }

        // Horizontal sum and add to Y[row]
        Y[row] += vaddvq_f32(acc);
    }

#else
    // Scalar fallback
    if (beta == 0.0f) {
        memset(Y, 0, M * sizeof(float));
    } else if (beta != 1.0f) {
        for (size_t i = 0; i < M; ++i) {
            Y[i] *= beta;
        }
    }

    size_t num_blocks_per_row = (K + block_size - 1) / block_size;

    for (size_t row = 0; row < M; ++row) {
        size_t byte_pos = row * ((K * 2 + 7) / 8);
        int bit_pos = 0;

        for (size_t block_idx = 0; block_idx < num_blocks_per_row; ++block_idx) {
            size_t k_start = block_idx * block_size;
            size_t k_end = std::min(k_start + block_size, K);
            float scale = A_scales[row * num_blocks_per_row + block_idx];

            for (size_t k = k_start; k < k_end; ++k) {
                uint8_t byte_val = A_quantized[byte_pos];
                uint8_t packed = (byte_val >> bit_pos) & 0b11;
                
                bit_pos += 2;
                if (bit_pos >= 8) {
                    byte_pos++;
                    bit_pos = 0;
                }

                int8_t ternary_val = (packed == 0b00) ? -1 : ((packed == 0b01) ? 0 : 1);
                Y[row] += ternary_val * scale * alpha * X[k];
            }
        }
    }
#endif
}

// ========== Quaternary (1.58-bit) Quantized GEMV ==========

void gemv_quaternary_1_58bit(
    size_t M,
    size_t K,
    float alpha,
    const uint8_t* A_quantized,
    const float* A_scales,
    const float* X,
    float beta,
    float* Y,
    size_t block_size
) {
    // Quaternary levels: {-1.5, -0.5, +0.5, +1.5}
    // Packed as 2 bits: 00=-1.5, 01=-0.5, 10=+0.5, 11=+1.5
    const float levels[4] = {-1.5f, -0.5f, 0.5f, 1.5f};

#ifdef KIPEPEO_NEON_ENABLED
    size_t num_blocks_per_row = (K + block_size - 1) / block_size;

    // Scale Y by beta
    if (beta == 0.0f) {
        memset(Y, 0, M * sizeof(float));
    } else if (beta != 1.0f) {
        float32x4_t beta_vec = vdupq_n_f32(beta);
        size_t i = 0;
        for (; i + 4 <= M; i += 4) {
            float32x4_t y_vec = vld1q_f32(&Y[i]);
            vst1q_f32(&Y[i], vmulq_f32(y_vec, beta_vec));
        }
        for (; i < M; ++i) {
            Y[i] *= beta;
        }
    }

    // Process rows
    for (size_t row = 0; row < M; ++row) {
        float32x4_t acc = vdupq_n_f32(0.0f);
        
        size_t byte_pos = row * ((K * 2 + 7) / 8);
        int bit_pos = 0;

        for (size_t block_idx = 0; block_idx < num_blocks_per_row; ++block_idx) {
            size_t k_start = block_idx * block_size;
            size_t k_end = std::min(k_start + block_size, K);
            float scale = A_scales[row * num_blocks_per_row + block_idx];

            float32x4_t scale_vec = vdupq_n_f32(scale * alpha);

            size_t k = k_start;
            for (; k + 4 <= k_end; k += 4) {
                float32x4_t x_vec = vld1q_f32(&X[k]);

                // Unpack 4 quaternary values
                float q_vals[4];
                for (int i = 0; i < 4; ++i) {
                    uint8_t byte_val = A_quantized[byte_pos];
                    uint8_t packed = (byte_val >> bit_pos) & 0b11;
                    
                    bit_pos += 2;
                    if (bit_pos >= 8) {
                        byte_pos++;
                        bit_pos = 0;
                    }

                    q_vals[i] = levels[packed];
                }

                float32x4_t q_vec = vld1q_f32(q_vals);
                q_vec = vmulq_f32(q_vec, scale_vec);
                acc = vfmaq_f32(acc, q_vec, x_vec);
            }

            // Remaining elements
            for (; k < k_end; ++k) {
                uint8_t byte_val = A_quantized[byte_pos];
                uint8_t packed = (byte_val >> bit_pos) & 0b11;
                
                bit_pos += 2;
                if (bit_pos >= 8) {
                    byte_pos++;
                    bit_pos = 0;
                }

                Y[row] += levels[packed] * scale * alpha * X[k];
            }
        }

        Y[row] += vaddvq_f32(acc);
    }

#else
    // Scalar fallback
    if (beta == 0.0f) {
        memset(Y, 0, M * sizeof(float));
    } else if (beta != 1.0f) {
        for (size_t i = 0; i < M; ++i) {
            Y[i] *= beta;
        }
    }

    size_t num_blocks_per_row = (K + block_size - 1) / block_size;

    for (size_t row = 0; row < M; ++row) {
        size_t byte_pos = row * ((K * 2 + 7) / 8);
        int bit_pos = 0;

        for (size_t block_idx = 0; block_idx < num_blocks_per_row; ++block_idx) {
            size_t k_start = block_idx * block_size;
            size_t k_end = std::min(k_start + block_size, K);
            float scale = A_scales[row * num_blocks_per_row + block_idx];

            for (size_t k = k_start; k < k_end; ++k) {
                uint8_t byte_val = A_quantized[byte_pos];
                uint8_t packed = (byte_val >> bit_pos) & 0b11;
                
                bit_pos += 2;
                if (bit_pos >= 8) {
                    byte_pos++;
                    bit_pos = 0;
                }

                Y[row] += levels[packed] * scale * alpha * X[k];
            }
        }
    }
#endif
}

// Stub implementations for other functions
void gemv_int8(size_t M, size_t K, float alpha, const int8_t* A_quantized,
               const float* A_scales, const float* X, float beta, float* Y) {
    // TODO: Standard INT8 GEMV implementation
}

void gemv_batch_1_28bit(size_t batch_size, size_t M, size_t K,
                       const uint8_t* A_quantized, const float* A_scales,
                       const float* X_batch, float* Y_batch, size_t block_size) {
    // Process each vector in batch sequentially
    for (size_t b = 0; b < batch_size; ++b) {
        gemv_ternary_1_28bit(M, K, 1.0f, A_quantized, A_scales,
                            &X_batch[b * K], 0.0f, &Y_batch[b * M], block_size);
    }
}

} // namespace neon
} // namespace kernels
} // namespace kipepeo
