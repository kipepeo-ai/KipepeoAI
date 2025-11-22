#include "kipepeo/kernels/mediatek/helio_optimizations.h"
#include "kipepeo/kernels/mediatek/helio_g85.h"
#include "kipepeo/kernels/mediatek/helio_g99.h"
#include "kipepeo/kernels/mediatek/helio_g100.h"
#include "kipepeo/kernels/neon/matrix_multiply.h"
#include "kipepeo/kernels/neon/quantized_gemm.h"

namespace kipepeo {
namespace kernels {
namespace mediatek {

void helio_matrix_multiply_f32(const float* A, const float* B, float* C,
                               size_t M, size_t N, size_t K, ChipType chip) {
    switch (chip) {
        case ChipType::MEDIATEK_HELIO_G85:
            helio_g85_matrix_multiply_f32(A, B, C, M, N, K);
            break;
        case ChipType::MEDIATEK_HELIO_G99:
            helio_g99_matrix_multiply_f32(A, B, C, M, N, K);
            break;
        case ChipType::MEDIATEK_HELIO_G100:
            helio_g100_matrix_multiply_f32(A, B, C, M, N, K);
            break;
        default:
            // Fallback to generic NEON
            neon::matrix_multiply_f32(A, B, C, M, N, K);
            break;
    }
}

void helio_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                               size_t M, size_t N, size_t K, ChipType chip) {
    switch (chip) {
        case ChipType::MEDIATEK_HELIO_G85:
            helio_g85_matrix_multiply_f16(A, B, C, M, N, K);
            break;
        case ChipType::MEDIATEK_HELIO_G99:
            helio_g99_matrix_multiply_f16(A, B, C, M, N, K);
            break;
        case ChipType::MEDIATEK_HELIO_G100:
            helio_g100_matrix_multiply_f16(A, B, C, M, N, K);
            break;
        default:
            neon::matrix_multiply_f16(A, B, C, M, N, K);
            break;
    }
}

void helio_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size, ChipType chip) {
    switch (chip) {
        case ChipType::MEDIATEK_HELIO_G85:
            helio_g85_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
        case ChipType::MEDIATEK_HELIO_G99:
            helio_g99_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
        case ChipType::MEDIATEK_HELIO_G100:
            helio_g100_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
        default:
            neon::gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
    }
}

void helio_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size, ChipType chip) {
    switch (chip) {
        case ChipType::MEDIATEK_HELIO_G85:
            helio_g85_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
        case ChipType::MEDIATEK_HELIO_G99:
            helio_g99_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
        case ChipType::MEDIATEK_HELIO_G100:
            helio_g100_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
        default:
            neon::gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
    }
}

} // namespace mediatek
} // namespace kernels
} // namespace kipepeo
