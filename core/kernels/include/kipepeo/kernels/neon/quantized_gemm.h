#pragma once

#include <cstddef>
#include <cstdint.h>

namespace kipepeo {
namespace kernels {
namespace neon {

/**
 * Specialized GEMM (General Matrix Multiply) for quantized weights
 * Optimized for 1.28-bit and 1.58-bit AfricaQuant quantization
 * 
 * These kernels use NEON intrinsics to accelerate quantized inference
 * on MediaTek Helio G99/G100, Unisoc T606, and Snapdragon 7s Gen 2
 */

/**
 * Ternary (1.28-bit) quantized GEMM
 * Computes: Y = alpha * A * X + beta * Y
 * where A is ternary quantized {-1, 0, +1}
 * 
 * @param M Number of rows in A
 * @param K Number of columns in A
 * @param alpha Scaling factor for A*X
 * @param A_quantized Quantized matrix A (packed ternary format)
 * @param A_scales Per-block scaling factors for A
 * @param X Input vector (K elements)
 * @param beta Scaling factor for Y
 * @param Y Output vector (M elements, in-place accumulation)
 * @param block_size Quantization block size
 */
void gemv_ternary_1_28bit(
    size_t M,
    size_t K,
    float alpha,
    const uint8_t* A_quantized,
    const float* A_scales,
    const float* X,
    float beta,
    float* Y,
    size_t block_size = 128
);

/**
 * Quaternary (1.58-bit) quantized GEMM
 * Computes: Y = alpha * A * X + beta * Y
 * where A is quaternary quantized {-1.5, -0.5, +0.5, +1.5}
 */
void gemv_quaternary_1_58bit(
    size_t M,
    size_t K,
    float alpha,
    const uint8_t* A_quantized,
    const float* A_scales,
    const float* X,
    float beta,
    float* Y,
    size_t block_size = 128
);

/**
 * INT8 quantized GEMM (for comparison/fallback)
 * Standard INT8 quantization is less efficient than AfricaQuant
 * but provided for compatibility
 */
void gemv_int8(
    size_t M,
    size_t K,
    float alpha,
    const int8_t* A_quantized,
    const float* A_scales,
    const float* X,
    float beta,
    float* Y
);

/**
 * Batch matrix-vector multiplication with 1.28-bit quantization
 * Processes multiple vectors at once for better throughput
 * 
 * @param batch_size Number of vectors to process
 * @param M Number of rows in A
 * @param K Number of columns in A
 * @param A_quantized Quantized matrix A (shared across batch)
 * @param A_scales Scaling factors for A
 * @param X_batch Input vectors (batch_size x K)
 * @param Y_batch Output vectors (batch_size x M)
 * @param block_size Quantization block size
 */
void gemv_batch_1_28bit(
    size_t batch_size,
    size_t M,
    size_t K,
    const uint8_t* A_quantized,
    const float* A_scales,
    const float* X_batch,
    float* Y_batch,
    size_t block_size = 128
);

} // namespace neon
} // namespace kernels
} // namespace kipepeo
