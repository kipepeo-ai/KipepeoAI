#pragma once

#include <cstddef>
#include <cstdint>

namespace kipepeo {
namespace kernels {
namespace apple {

/**
 * Apple Silicon CPU NEON optimizations
 * Architecture: Firestorm (big) + Icestorm (little) cores
 * Cache: Large unified cache, high memory bandwidth
 * Features: Native FP16, aggressive blocking possible
 * 
 * Note: Metal backend exists for GPU acceleration.
 * These are CPU-side NEON optimizations.
 */

// FP32 matrix multiplication optimized for Apple Silicon
void apple_neon_matrix_multiply_f32(const float* A, const float* B, float* C,
                                    size_t M, size_t N, size_t K);

// FP16 matrix multiplication with native FP16 support
void apple_neon_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                                    size_t M, size_t N, size_t K);

// Quantized GEMM for 1.28-bit (ternary)
void apple_neon_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size = 128);

// Quantized GEMM for 1.58-bit (quaternary)
void apple_neon_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size = 128);

} // namespace apple
} // namespace kernels
} // namespace kipepeo

