#pragma once

#include "kipepeo/kernels/chip_detection.h"
#include <cstddef>

namespace kipepeo {
namespace kernels {
namespace mediatek {

/**
 * MediaTek Helio series optimizations
 * Provides chip-specific kernel dispatch for Helio G85, G99, and G100
 */

// Unified matrix multiply dispatch - selects appropriate Helio variant
void helio_matrix_multiply_f32(const float* A, const float* B, float* C,
                               size_t M, size_t N, size_t K, ChipType chip);

void helio_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                               size_t M, size_t N, size_t K, ChipType chip);

// Unified quantized GEMM dispatch
void helio_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size, ChipType chip);

void helio_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size, ChipType chip);

} // namespace mediatek
} // namespace kernels
} // namespace kipepeo

