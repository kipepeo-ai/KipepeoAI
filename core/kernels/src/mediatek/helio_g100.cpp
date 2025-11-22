#include "kipepeo/kernels/mediatek/helio_g100.h"
#include "kipepeo/kernels/mediatek/helio_g99.h"
#include <cstring>

namespace kipepeo {
namespace kernels {
namespace mediatek {

// Helio G100 is similar to G99, reuse G99 implementations
// Can be fine-tuned later if performance characteristics differ

void helio_g100_matrix_multiply_f32(const float* A, const float* B, float* C,
                                    size_t M, size_t N, size_t K) {
    // G100 uses same architecture as G99
    helio_g99_matrix_multiply_f32(A, B, C, M, N, K);
}

void helio_g100_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                                    size_t M, size_t N, size_t K) {
    helio_g99_matrix_multiply_f16(A, B, C, M, N, K);
}

void helio_g100_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size) {
    helio_g99_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
}

void helio_g100_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size) {
    helio_g99_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
}

} // namespace mediatek
} // namespace kernels
} // namespace kipepeo

