#include "kipepeo/kernels/kernel_dispatch.h"
#include "kipepeo/kernels/chip_detection.h"
#include "kipepeo/kernels/mediatek/helio_optimizations.h"
#include "kipepeo/kernels/qualcomm/snapdragon_common.h"
#include "kipepeo/kernels/unisoc/t606.h"
#include "kipepeo/kernels/apple/neon_apple.h"
#include "kipepeo/kernels/neon/matrix_multiply.h"
#include "kipepeo/kernels/neon/quantized_gemm.h"

namespace kipepeo {
namespace kernels {

namespace {
    // Cache detected chip type
    ChipType g_chip = ChipType::UNKNOWN;
    bool g_chip_initialized = false;
    
    ChipType get_chip() {
        if (!g_chip_initialized) {
            g_chip = detect_chip();
            g_chip_initialized = true;
        }
        return g_chip;
    }
} // anonymous namespace

void matrix_multiply_f32_chip_optimized(const float* A, const float* B, float* C,
                                        size_t M, size_t N, size_t K) {
    ChipType chip = get_chip();
    
    switch (chip) {
        // MediaTek Helio series
        case ChipType::MEDIATEK_HELIO_G85:
        case ChipType::MEDIATEK_HELIO_G99:
        case ChipType::MEDIATEK_HELIO_G100:
            mediatek::helio_matrix_multiply_f32(A, B, C, M, N, K, chip);
            break;
            
        // Qualcomm Snapdragon series
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            qualcomm::snapdragon_matrix_multiply_f32(A, B, C, M, N, K, chip);
            break;
            
        // Unisoc series
        case ChipType::UNISOC_T606:
            unisoc::t606_matrix_multiply_f32(A, B, C, M, N, K);
            break;
            
        // Apple Silicon
        case ChipType::APPLE_A13:
        case ChipType::APPLE_A14:
        case ChipType::APPLE_A15:
        case ChipType::APPLE_A16:
        case ChipType::APPLE_A17:
        case ChipType::APPLE_A18:
        case ChipType::APPLE_M1:
        case ChipType::APPLE_M2:
        case ChipType::APPLE_M3:
        case ChipType::APPLE_M4:
            apple::apple_neon_matrix_multiply_f32(A, B, C, M, N, K);
            break;
            
        default:
            // Fallback to generic NEON
            neon::matrix_multiply_f32(A, B, C, M, N, K);
            break;
    }
}

void matrix_multiply_f16_chip_optimized(const __fp16* A, const __fp16* B, __fp16* C,
                                        size_t M, size_t N, size_t K) {
    ChipType chip = get_chip();
    
    switch (chip) {
        // MediaTek Helio series
        case ChipType::MEDIATEK_HELIO_G85:
        case ChipType::MEDIATEK_HELIO_G99:
        case ChipType::MEDIATEK_HELIO_G100:
            mediatek::helio_matrix_multiply_f16(A, B, C, M, N, K, chip);
            break;
            
        // Qualcomm Snapdragon series
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            qualcomm::snapdragon_matrix_multiply_f16(A, B, C, M, N, K, chip);
            break;
            
        // Unisoc series
        case ChipType::UNISOC_T606:
            unisoc::t606_matrix_multiply_f16(A, B, C, M, N, K);
            break;
            
        // Apple Silicon
        case ChipType::APPLE_A13:
        case ChipType::APPLE_A14:
        case ChipType::APPLE_A15:
        case ChipType::APPLE_A16:
        case ChipType::APPLE_A17:
        case ChipType::APPLE_A18:
        case ChipType::APPLE_M1:
        case ChipType::APPLE_M2:
        case ChipType::APPLE_M3:
        case ChipType::APPLE_M4:
            apple::apple_neon_matrix_multiply_f16(A, B, C, M, N, K);
            break;
            
        default:
            neon::matrix_multiply_f16(A, B, C, M, N, K);
            break;
    }
}

void gemv_ternary_1_28bit_chip_optimized(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size) {
    ChipType chip = get_chip();
    
    switch (chip) {
        // MediaTek Helio series
        case ChipType::MEDIATEK_HELIO_G85:
        case ChipType::MEDIATEK_HELIO_G99:
        case ChipType::MEDIATEK_HELIO_G100:
            mediatek::helio_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size, chip);
            break;
            
        // Qualcomm Snapdragon series
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            qualcomm::snapdragon_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size, chip);
            break;
            
        // Unisoc series
        case ChipType::UNISOC_T606:
            unisoc::t606_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
            
        // Apple Silicon
        case ChipType::APPLE_A13:
        case ChipType::APPLE_A14:
        case ChipType::APPLE_A15:
        case ChipType::APPLE_A16:
        case ChipType::APPLE_A17:
        case ChipType::APPLE_A18:
        case ChipType::APPLE_M1:
        case ChipType::APPLE_M2:
        case ChipType::APPLE_M3:
        case ChipType::APPLE_M4:
            apple::apple_neon_gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
            
        default:
            neon::gemv_ternary_1_28bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
    }
}

void gemv_quaternary_1_58bit_chip_optimized(
    size_t M, size_t K, float alpha,
    const uint8_t* A_quantized, const float* A_scales,
    const float* X, float beta, float* Y, size_t block_size) {
    ChipType chip = get_chip();
    
    switch (chip) {
        // MediaTek Helio series
        case ChipType::MEDIATEK_HELIO_G85:
        case ChipType::MEDIATEK_HELIO_G99:
        case ChipType::MEDIATEK_HELIO_G100:
            mediatek::helio_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size, chip);
            break;
            
        // Qualcomm Snapdragon series
        case ChipType::QUALCOMM_SNAPDRAGON_7S_GEN2:
            qualcomm::snapdragon_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size, chip);
            break;
            
        // Unisoc series
        case ChipType::UNISOC_T606:
            unisoc::t606_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
            
        // Apple Silicon
        case ChipType::APPLE_A13:
        case ChipType::APPLE_A14:
        case ChipType::APPLE_A15:
        case ChipType::APPLE_A16:
        case ChipType::APPLE_A17:
        case ChipType::APPLE_A18:
        case ChipType::APPLE_M1:
        case ChipType::APPLE_M2:
        case ChipType::APPLE_M3:
        case ChipType::APPLE_M4:
            apple::apple_neon_gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
            
        default:
            neon::gemv_quaternary_1_58bit(M, K, alpha, A_quantized, A_scales, X, beta, Y, block_size);
            break;
    }
}

} // namespace kernels
} // namespace kipepeo

