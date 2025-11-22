#pragma once

#include <cstdint>
#include <cstddef>

namespace kipepeo {
namespace kernels {

/**
 * Chip type enumeration for runtime chip detection
 */
enum class ChipType : uint8_t {
    UNKNOWN = 0,
    
    // MediaTek Helio series
    MEDIATEK_HELIO_G85,
    MEDIATEK_HELIO_G99,
    MEDIATEK_HELIO_G100,
    
    // Qualcomm Snapdragon series
    QUALCOMM_SNAPDRAGON_7S_GEN2,
    
    // Unisoc series
    UNISOC_T606,
    
    // Apple Silicon (A-series)
    APPLE_A13,
    APPLE_A14,
    APPLE_A15,
    APPLE_A16,
    APPLE_A17,
    APPLE_A18,
    
    // Apple Silicon (M-series)
    APPLE_M1,
    APPLE_M2,
    APPLE_M3,
    APPLE_M4
};

/**
 * Detect the current chip type at runtime
 * Uses /proc/cpuinfo on Android/Linux and system properties
 * Returns ChipType::UNKNOWN if detection fails
 */
ChipType detect_chip();

/**
 * Get human-readable chip name string
 * @param chip The chip type
 * @return String representation of the chip name
 */
const char* get_chip_name(ChipType chip);

/**
 * Check if chip supports FP16 native operations
 * @param chip The chip type
 * @return true if chip has native FP16 support
 */
bool chip_supports_fp16(ChipType chip);

/**
 * Get optimal matrix multiplication block size for the chip
 * @param chip The chip type
 * @param is_big_core Whether this is a big core (true) or little core (false)
 * @return Optimal block size (MR x NR)
 */
void get_optimal_block_size(ChipType chip, bool is_big_core, size_t& MR, size_t& NR);

} // namespace kernels
} // namespace kipepeo

