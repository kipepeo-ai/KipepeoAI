#include "kipepeo/quantization/hardware_detection.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdio>

#ifdef __ANDROID__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

#ifdef KIPEPEO_NEON_ENABLED
#include <arm_neon.h>
#endif

namespace kipepeo {
namespace quantization {

// Static storage for CPU model
static char cpu_model_buffer[128] = "Unknown";

HardwareCapabilities detect_hardware_capabilities() {
    HardwareCapabilities caps = {};
    
    // Detect NEON support
#ifdef KIPEPEO_NEON_ENABLED
    caps.has_neon = true;
#else
    caps.has_neon = false;
#endif
    
    // Detect FP16 support (ARMv8.2+)
    caps.has_fp16 = false; // Conservative default
    
    // Cache sizes - use conservative defaults for low-end devices
    // These are typical values for MediaTek Helio G99/G100, Unisoc T606
    caps.l1_cache_size = 32 * 1024;  // 32KB L1 cache (typical)
    caps.l2_cache_size = 256 * 1024; // 256KB L2 cache (typical)
    caps.l3_cache_size = 0;          // No L3 cache on low-end devices
    
    // Memory detection
#ifdef __ANDROID__
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        caps.total_memory = info.totalram * info.mem_unit;
        caps.available_memory = info.freeram * info.mem_unit;
    } else {
        // Fallback: assume 2GB total, 512MB available
        caps.total_memory = 2ULL * 1024 * 1024 * 1024;
        caps.available_memory = 512ULL * 1024 * 1024;
    }
    
    // CPU cores
    caps.cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (caps.cpu_cores <= 0) {
        caps.cpu_cores = 4; // Conservative default
    }
#else
    // Desktop/development defaults
    caps.total_memory = 8ULL * 1024 * 1024 * 1024;
    caps.available_memory = 4ULL * 1024 * 1024 * 1024;
    caps.cpu_cores = 4;
#endif
    
    // CPU model detection (simplified - can be enhanced)
    const char* cpuinfo_path = "/proc/cpuinfo";
    FILE* f = fopen(cpuinfo_path, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "Hardware", 8) == 0 || 
                strncmp(line, "model name", 10) == 0) {
                // Extract model name
                char* colon = strchr(line, ':');
                if (colon) {
                    colon++;
                    while (*colon == ' ' || *colon == '\t') colon++;
                    size_t len = strlen(colon);
                    while (len > 0 && (colon[len-1] == '\n' || colon[len-1] == '\r')) {
                        colon[--len] = '\0';
                    }
                    strncpy(cpu_model_buffer, colon, sizeof(cpu_model_buffer) - 1);
                    cpu_model_buffer[sizeof(cpu_model_buffer) - 1] = '\0';
                    break;
                }
            }
        }
        fclose(f);
    }
    caps.cpu_model = cpu_model_buffer;
    
    // Determine optimal settings based on hardware
    if (caps.available_memory < 1ULL * 1024 * 1024 * 1024) {
        // Very low memory (<1GB): use smaller blocks
        caps.optimal_block_size = 64;
        caps.use_memory_pooling = true;
        caps.max_concurrent_ops = 1;
    } else if (caps.available_memory < 2ULL * 1024 * 1024 * 1024) {
        // Low memory (1-2GB): use medium blocks
        caps.optimal_block_size = 128;
        caps.use_memory_pooling = true;
        caps.max_concurrent_ops = 2;
    } else {
        // Adequate memory (>2GB): use larger blocks
        caps.optimal_block_size = 256;
        caps.use_memory_pooling = false;
        caps.max_concurrent_ops = 4;
    }
    
    // Adaptive threshold based on hardware
    // Lower-end devices may benefit from slightly different thresholds
    caps.optimal_threshold_1_28 = 0.33f; // Default, can be adjusted per model
    
    return caps;
}

uint32_t get_optimal_block_size(size_t model_size, size_t available_memory) {
    // For very large models (>10B parameters), use larger blocks if memory allows
    if (model_size > 10ULL * 1000 * 1000 * 1000) {
        if (available_memory > 4ULL * 1024 * 1024 * 1024) {
            return 256;
        }
        return 128;
    }
    
    // For medium models (1B-10B), use medium blocks
    if (model_size > 1ULL * 1000 * 1000 * 1000) {
        if (available_memory > 2ULL * 1024 * 1024 * 1024) {
            return 256;
        }
        return 128;
    }
    
    // For small models (<1B), use smaller blocks to reduce overhead
    if (available_memory < 1ULL * 1024 * 1024 * 1024) {
        return 64;
    }
    return 128;
}

float get_adaptive_threshold_1_28(const float* weights, size_t count,
                                  const HardwareCapabilities& hardware) {
    if (!weights || count == 0) {
        return hardware.optimal_threshold_1_28;
    }
    
    // Compute weight distribution statistics
    float sum = 0.0f;
    float sum_sq = 0.0f;
    float max_abs = 0.0f;
    
    size_t sample_size = std::min(count, size_t(10000)); // Sample for efficiency
    size_t step = count / sample_size;
    
    for (size_t i = 0; i < count; i += step) {
        float val = weights[i];
        float abs_val = std::fabs(val);
        sum += val;
        sum_sq += val * val;
        max_abs = std::max(max_abs, abs_val);
    }
    
    float mean = sum / sample_size;
    float variance = (sum_sq / sample_size) - (mean * mean);
    float std_dev = std::sqrt(std::max(0.0f, variance));
    
    // Adaptive threshold based on distribution
    // For distributions with high variance, use higher threshold
    // For tight distributions, use lower threshold
    float adaptive_threshold = 0.33f;
    
    if (max_abs > 0.0f) {
        float coefficient_of_variation = std_dev / max_abs;
        
        // Adjust threshold based on CV
        if (coefficient_of_variation > 0.5f) {
            // High variance: use higher threshold to capture more signal
            adaptive_threshold = 0.35f;
        } else if (coefficient_of_variation < 0.2f) {
            // Low variance: use lower threshold for better precision
            adaptive_threshold = 0.28f;
        }
    }
    
    return adaptive_threshold;
}

bool is_neon_aligned(const void* ptr) {
    return (reinterpret_cast<uintptr_t>(ptr) % 16) == 0;
}

const char* get_cpu_model() {
    return cpu_model_buffer;
}

} // namespace quantization
} // namespace kipepeo

