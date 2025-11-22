#include "kipepeo/kernels/unisoc/t606.h"
#include "kipepeo/kernels/neon/matrix_multiply.h"
#include "kipepeo/kernels/neon/quantized_gemm.h"
#include <cstring>

#ifdef KIPEPEO_NEON_ENABLED
#include <arm_neon.h>
#endif

namespace kipepeo {
namespace kernels {
namespace unisoc {

// Unisoc T606: Cortex-A75/A55, conservative 4x4 blocking for cache efficiency
void t606_matrix_multiply_f32(const float* A, const float* B, float* C,
                              size_t M, size_t N, size_t K) {
#ifdef KIPEPEO_NEON_ENABLED
    // Conservative blocking for smaller cache - maximize cache reuse
    const size_t MR = 4;
    const size_t NR = 4;
    
    memset(C, 0, M * N * sizeof(float));
    
    for (size_t i = 0; i < M; i += MR) {
        size_t m_block = (i + MR <= M) ? MR : (M - i);
        
        for (size_t j = 0; j < N; j += NR) {
            size_t n_block = (j + NR <= N) ? NR : (N - j);
            
            // Accumulator registers for 4x4 block
            float32x4_t acc[4][4];
            for (size_t ii = 0; ii < 4; ++ii) {
                for (size_t jj = 0; jj < 4; ++jj) {
                    acc[ii][jj] = vdupq_n_f32(0.0f);
                }
            }
            
            // Inner loop - focus on cache reuse over prefetching
            size_t k = 0;
            for (; k + 4 <= K; k += 4) {
                // Load A vectors
                float32x4_t a_vec[4];
                for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                    a_vec[ii] = vld1q_f32(&A[(i + ii) * K + k]);
                }
                
                // Process B columns - minimize memory accesses
                for (size_t jj = 0; jj < n_block && (j + jj) < N; ++jj) {
                    float b_vals[4];
                    for (int kk = 0; kk < 4 && (k + kk) < K; ++kk) {
                        b_vals[kk] = B[(k + kk) * N + (j + jj)];
                    }
                    float32x4_t b_vec = vld1q_f32(b_vals);
                    
                    // FMA for each row
                    for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                        acc[ii][jj] = vfmaq_f32(acc[ii][jj], a_vec[ii], b_vec);
                    }
                }
            }
            
            // Handle remaining K elements
            for (; k < K; ++k) {
                for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                    float a_val = A[(i + ii) * K + k];
                    for (size_t jj = 0; jj < n_block && (j + jj) < N; ++jj) {
                        float b_val = B[k * N + (j + jj)];
                        C[(i + ii) * N + (j + jj)] += a_val * b_val;
                    }
                }
            }
            
            // Store results
            for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                for (size_t jj = 0; jj < n_block && (j + jj) < N; ++jj) {
                    float sum = vaddvq_f32(acc[ii][jj]);
                    C[(i + ii) * N + (j + jj)] += sum;
                }
            }
        }
    }
#else
    neon::matrix_multiply_f32(A, B, C, M, N, K);
#endif
}

void t606_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                              size_t M, size_t N, size_t K) {
    // T606 may not have native FP16, fallback to generic
    neon::matrix_multiply_f16(A, B, C, M, N, K);
}

void t606_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size) {
    // Use generic NEON implementation - T606 benefits from cache-efficient approach
    neon::gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
}

void t606_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size) {
    neon::gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
}

} // namespace unisoc
} // namespace kernels
} // namespace kipepeo

