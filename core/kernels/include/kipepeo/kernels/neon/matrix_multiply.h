#pragma once

#include <cstddef>

namespace kipepeo {
namespace kernels {
namespace neon {

/**
 * Optimized matrix multiplication using ARM NEON intrinsics
 * Provides significant speedup on MediaTek Helio G85/G99, Unisoc T606, Snapdragon 7s Gen 2
 */

// FP32 matrix multiplication: C = A * B
// A: M x K, B: K x N, C: M x N
void matrix_multiply_f32(const float* A, const float* B, float* C,
                        size_t M, size_t N, size_t K);

// FP16 matrix multiplication (for newer ARM chips with FP16 support)
void matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                        size_t M, size_t N, size_t K);

// Quantized matrix multiplication variants

/**
 * Matrix-vector multiplication with 1.28-bit quantized matrix
 * Y = A * X, where A is 1.28-bit quantized (ternary), X and Y are FP32
 * 
 * @param A_quantized Quantized matrix A (M x K, packed ternary)
 * @param A_scales Scaling factors per block
 * @param X Input vector (K elements, FP32)
 * @param Y Output vector (M elements, FP32)
 * @param M Number of rows in A
 * @param K Number of columns in A (must equal length of X)
 * @param block_size Quantization block size
 */
void matvec_mul_1_28bit(
    const uint8_t* A_quantized,
    const float* A_scales,
    const float* X,
    float* Y,
    size_t M,
    size_t K,
    size_t block_size = 128
);

/**
 * Matrix-vector multiplication with 1.58-bit quantized matrix
 * Y = A * X, where A is 1.58-bit quantized (quaternary), X and Y are FP32
 */
void matvec_mul_1_58bit(
    const uint8_t* A_quantized,
    const float* A_scales,
    const float* X,
    float* Y,
    size_t M,
    size_t K,
    size_t block_size = 128
);

/**
 * Mixed-precision matrix multiplication
 * Automatically selects best precision based on available hardware
 */
void matrix_multiply_mixed_precision(
    const void* A,
    const void* B,
    void* C,
    size_t M,
    size_t N,
    size_t K,
    bool use_fp16 = false
);

} // namespace neon
} // namespace kernels
} // namespace kipepeo
