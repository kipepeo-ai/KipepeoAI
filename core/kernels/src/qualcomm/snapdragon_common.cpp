#include "kipepeo/kernels/qualcomm/snapdragon_common.h"
#include "kipepeo/kernels/qualcomm/snapdragon_7s_gen2.h"
#include "kipepeo/kernels/neon/matrix_multiply.h"
#include "kipepeo/kernels/neon/quantized_gemm.h"

namespace kipepeo {
namespace kernels {
namespace qualcomm {

void snapdragon_matrix_multiply_f32(const float* A, const float* B, float* C,
                                    size_t M, size_t N, size_t K, ChipType chip) {
    switch (chip) {
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            snapdragon_7s_gen2_matrix_multiply_f32(A, B, C, M, N, K);
            break;
        default:
            // Fallback to generic NEON
            neon::matrix_multiply_f32(A, B, C, M, N, K);
            break;
    }
}

void snapdragon_matrix_multiply_f16(const __fp16* A, const __fp16* B, __fp16* C,
                                    size_t M, size_t N, size_t K, ChipType chip) {
    switch (chip) {
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            snapdragon_7s_gen2_matrix_multiply_f16(A, B, C, M, N, K);
            break;
        default:
            neon::matrix_multiply_f16(A, B, C, M, N, K);
            break;
    }
}

void snapdragon_gemv_ternary_1_28bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size, ChipType chip) {
    switch (chip) {
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            snapdragon_7s_gen2_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
        default:
            neon::gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
    }
}

void snapdragon_gemv_quaternary_1_58bit(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size, ChipType chip) {
    switch (chip) {
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            snapdragon_7s_gen2_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
        default:
            neon::gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
    }
}

} // namespace qualcomm
} // namespace kernels
} // namespace kipepeo

