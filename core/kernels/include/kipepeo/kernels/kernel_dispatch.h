#pragma once

#include "kipepeo/kernels/chip_detection.h"
#include <cstddef>
#include <cstdint>

namespace kipepeo {
namespace kernels {

/**
 * Unified kernel dispatch system
 * Automatically selects chip-specific kernels at runtime
 * Falls back to generic NEON if chip-specific kernel not available
 */

// Matrix multiplication dispatch
void matrix_multiply_f32_chip_optimized(const float* A, const float* B, float* C,
                                        size_t M, size_t N, size_t K);

void matrix_multiply_f16_chip_optimized(const __fp16* A, const __fp16* B, __fp16* C,
                                        size_t M, size_t N, size_t K);

// Quantized GEMM dispatch
void gemv_ternary_1_28bit_chip_optimized(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size = 128);

void gemv_quaternary_1_58bit_chip_optimized(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size = 128);

} // namespace kernels
} // namespace kipepeo

