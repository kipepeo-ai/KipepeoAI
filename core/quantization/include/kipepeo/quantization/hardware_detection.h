#pragma once

#include <cstddef>
#include <cstdint>

namespace kipepeo {
namespace quantization {

/**
 * Hardware capabilities detected at runtime
 */
struct HardwareCapabilities {
    bool has_neon;
    bool has_fp16;
    size_t l1_cache_size;      // L1 cache size in bytes
    size_t l2_cache_size;      // L2 cache size in bytes
    size_t l3_cache_size;      // L3 cache size in bytes (0 if not available)
    size_t total_memory;        // Total available memory in bytes
    size_t available_memory;    // Available memory in bytes
    uint32_t cpu_cores;         // Number of CPU cores
    const char* cpu_model;      // CPU model string (e.g., "MediaTek Helio G99")
    
    // Optimal settings based on hardware
    uint32_t optimal_block_size;    // Optimal block size for quantization
    float optimal_threshold_1_28;    // Optimal threshold for 1.28-bit quantization
    bool use_memory_pooling;         // Whether to use memory pooling
    uint32_t max_concurrent_ops;      // Maximum concurrent operations
};

/**
 * Detect hardware capabilities and return optimal settings
 * This function should be called once at startup
 */
HardwareCapabilities detect_hardware_capabilities();

/**
 * Get optimal block size for given model size and hardware
 * @param model_size Total number of weights in the model
 * @param available_memory Available memory in bytes
 * @return Optimal block size (64, 128, or 256)
 */
uint32_t get_optimal_block_size(size_t model_size, size_t available_memory);

/**
 * Get adaptive threshold for 1.28-bit quantization based on weight distribution
 * @param weights Weight array
 * @param count Number of weights
 * @param hardware Hardware capabilities
 * @return Optimal threshold (typically 0.25-0.4)
 */
float get_adaptive_threshold_1_28(const float* weights, size_t count, 
                                   const HardwareCapabilities& hardware);

/**
 * Check if memory is aligned for NEON operations (16-byte alignment)
 */
bool is_neon_aligned(const void* ptr);

/**
 * Get CPU model string (e.g., "MediaTek Helio G99")
 */
const char* get_cpu_model();

} // namespace quantization
} // namespace kipepeo

