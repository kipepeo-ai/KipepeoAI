#pragma once

#include <cstddef>
#include <cstdint>

namespace kipepeo {
namespace kernels {
namespace qualcomm {

/**
 * Qualcomm Snapdragon 7s Gen 2 specific optimizations
 * Architecture: ARM Cortex-A78 (big) + Cortex-A55 (little)
 * Cache: 64KB L1 I/D, 512KB L2 per cluster
 * Features: Native FP16 support, ARMv8.2+ features
 */

// FP32 matrix multiplication optimized for Snapdragon 7s Gen 2
void snapdragon_7s_gen2_matrix_multiply_f32(const float* A, const float* B, float* C,
                                           size_t M, size_t N, size_t K);

// FP16 matrix multiplication with native FP16 support
void snapdragon_7s_gen2_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                                           size_t M, size_t N, size_t K);

// Quantized GEMM for 1.28-bit (ternary)
void snapdragon_7s_gen2_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size = 128);

// Quantized GEMM for 1.58-bit (quaternary)
void snapdragon_7s_gen2_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size = 128);

} // namespace qualcomm
} // namespace kernels
} // namespace kipepeo

