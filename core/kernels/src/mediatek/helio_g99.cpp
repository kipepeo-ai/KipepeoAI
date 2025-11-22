#include "kipepeo/kernels/mediatek/helio_g99.h"
#include "kipepeo/kernels/neon/matrix_multiply.h"
#include "kipepeo/kernels/neon/quantized_gemm.h"
#include <cstring>

#ifdef KIPEPEO_NEON_ENABLED
#include <arm_neon.h>
#endif

namespace kipepeo {
namespace kernels {
namespace mediatek {

// Helio G99: Cortex-A76/A55, 6x6 blocking for big cores, 4x4 for little cores
void helio_g99_matrix_multiply_f32(const float* A, const float* B, float* C,
                                   size_t M, size_t N, size_t K) {
#ifdef KIPEPEO_NEON_ENABLED
    // Optimized for Cortex-A76 with 6x6 blocking (can use 4x4 for A55)
    const size_t MR = 6;  // Big core blocking
    const size_t NR = 6;
    
    memset(C, 0, M * N * sizeof(float));
    
    for (size_t i = 0; i < M; i += MR) {
        size_t m_block = (i + MR <= M) ? MR : (M - i);
        
        for (size_t j = 0; j < N; j += NR) {
            size_t n_block = (j + NR <= N) ? NR : (N - j);
            
            // Accumulator registers for 6x6 block
            float32x4_t acc[6][6];
            for (size_t ii = 0; ii < 6; ++ii) {
                for (size_t jj = 0; jj < 6; ++jj) {
                    acc[ii][jj] = vdupq_n_f32(0.0f);
                }
            }
            
            // Inner loop with software pipelining
            size_t k = 0;
            for (; k + 4 <= K; k += 4) {
                // Load A vectors (6 rows)
                float32x4_t a_vec[6];
                for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                    a_vec[ii] = vld1q_f32(&A[(i + ii) * K + k]);
                }
                
                // Process B columns with prefetching
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

void helio_g99_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                                   size_t M, size_t N, size_t K) {
#ifdef KIPEPEO_NEON_ENABLED
    // G99 has native FP16 support - use 8x8 blocking
    const size_t MR = 8;
    const size_t NR = 8;
    
    memset(C, 0, M * N * sizeof(__fp16));
    
    for (size_t i = 0; i < M; i += MR) {
        size_t m_block = (i + MR <= M) ? MR : (M - i);
        
        for (size_t j = 0; j < N; j += NR) {
            size_t n_block = (j + NR <= N) ? NR : (N - j);
            
            float16x8_t acc[8][8];
            for (size_t ii = 0; ii < 8; ++ii) {
                for (size_t jj = 0; jj < 8; ++jj) {
                    acc[ii][jj] = vdupq_n_f16(0.0f);
                }
            }
            
            size_t k = 0;
            for (; k + 8 <= K; k += 8) {
                float16x8_t a_vec[8];
                for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                    a_vec[ii] = vld1q_f16(&A[(i + ii) * K + k]);
                }
                
                for (size_t jj = 0; jj < n_block && (j + jj) < N; ++jj) {
                    __fp16 b_vals[8];
                    for (int kk = 0; kk < 8 && (k + kk) < K; ++kk) {
                        b_vals[kk] = B[(k + kk) * N + (j + jj)];
                    }
                    float16x8_t b_vec = vld1q_f16(b_vals);
                    
                    for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                        acc[ii][jj] = vfmaq_f16(acc[ii][jj], a_vec[ii], b_vec);
                    }
                }
            }
            
            for (; k < K; ++k) {
                for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                    __fp16 a_val = A[(i + ii) * K + k];
                    for (size_t jj = 0; jj < n_block && (j + jj) < N; ++jj) {
                        __fp16 b_val = B[k * N + (j + jj)];
                        C[(i + ii) * N + (j + jj)] += a_val * b_val;
                    }
                }
            }
            
            for (size_t ii = 0; ii < m_block && (i + ii) < M; ++ii) {
                for (size_t jj = 0; jj < n_block && (j + jj) < N; ++jj) {
                    __fp16 sum = vaddvq_f16(acc[ii][jj]);
                    C[(i + ii) * N + (j + jj)] += sum;
                }
            }
        }
    }
#else
    neon::matrix_multiply_f16(A, B, C, M, N, K);
#endif
}

void helio_g99_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size) {
    // G99 supports ARMv8.2 dot product - can optimize further
    // For now, use generic with potential dot product hints
    neon::gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
}

void helio_g99_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size) {
    neon::gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
}

} // namespace mediatek
} // namespace kernels
} // namespace kipepeo

