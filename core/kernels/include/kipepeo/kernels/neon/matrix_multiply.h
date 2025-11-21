#pragma once

#include <cstddef>

namespace kipepeo {
namespace kernels {
namespace neon {

/**
 * Optimized matrix multiplication using ARM NEON intrinsics
 * Provides 2x speedup on MediaTek Helio G85/G99 chips
 */
void matrix_multiply_f32(const float* A, const float* B, float* C,
                        size_t M, size_t N, size_t K);

void matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                        size_t M, size_t N, size_t K);

} // namespace neon
} // namespace kernels
} // namespace kipepeo

