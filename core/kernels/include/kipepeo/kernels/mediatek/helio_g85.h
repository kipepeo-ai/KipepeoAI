#pragma once

#include <cstddef>
#include <cstdint>

namespace kipepeo {
namespace kernels {
namespace mediatek {

/**
 * MediaTek Helio G85 specific optimizations
 * Architecture: ARM Cortex-A75 (big) + Cortex-A55 (little)
 * Cache: 64KB L1 I/D, 256KB L2 per cluster
 */

// FP32 matrix multiplication optimized for Helio G85
void helio_g85_matrix_multiply_f32(const float* A, const float* B, float* C,
                                   size_t M, size_t N, size_t K);

// FP16 matrix multiplication (if supported)
void helio_g85_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                                   size_t M, size_t N, size_t K);

// Quantized GEMM for 1.28-bit (ternary)
void helio_g85_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size = 128);

// Quantized GEMM for 1.58-bit (quaternary)
void helio_g85_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size = 128);

} // namespace mediatek
} // namespace kernels
} // namespace kipepeo

