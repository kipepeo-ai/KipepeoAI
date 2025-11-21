#pragma once

#include <cstdint.h>
#include <string>

namespace kipepeo {
namespace llm {

/**
 * Dynamic Model Switcher
 * 
 * Automatically switches between model sizes (7B → 13B → 34B → 70B)
 * based on available system RAM. Enables running larger models on
 * low-end devices by gracefully falling back when memory is constrained.
 */

enum class ModelSize {
    MODEL_7B = 0,
    MODEL_13B = 1,
    MODEL_34B = 2,
    MODEL_70B = 3,
    MODEL_UNKNOWN = 255
};

struct ModelInfo {
    ModelSize size;
    std::string model_path;
    uint64_t required_ram_mb;    // Estimated RAM requirement in MB
    uint64_t optimal_ram_mb;     // Optimal RAM for good performance
    bool is_loaded;
};

struct SystemMemoryInfo {
    uint64_t total_ram_mb;       // Total system RAM
    uint64_t available_ram_mb;   // Currently available RAM
    uint64_t free_ram_mb;        // Free (unused) RAM
    float usage_percent;         // RAM usage percentage
};

class ModelSwitcher {
public:
    ModelSwitcher();
    ~ModelSwitcher();

    /**
     * Register a model for automatic switching
     * Models should be registered from smallest to largest
     */
    bool register_model(ModelSize size, const std::string& path,
                       uint64_t required_ram_mb, uint64_t optimal_ram_mb);

    /**
     * Select the best model based on current available RAM
     * 
     * @param min_free_ram_mb Minimum free RAM to maintain (default: 1024 MB)
     * @return Best model size for current RAM conditions
     */
    ModelSize select_best_model(uint64_t min_free_ram_mb = 1024);

    /**
     * Get current system memory info (reads /proc/meminfo on Android)
     */
    static SystemMemoryInfo get_memory_info();

    /**
     * Get available RAM in GB (convenience function)
     */
    static float get_available_ram_gb();

    /**
     * Check if we should switch to a smaller model due to memory pressure
     */
    bool should_downgrade(ModelSize current_size, uint64_t min_free_ram_mb = 1024);

    /**
     * Check if we can upgrade to a larger model
     */
    bool can_upgrade(ModelSize current_size, uint64_t min_free_ram_mb = 1024);

    /**
     * Get model info for a given size
     */
    const ModelInfo* get_model_info(ModelSize size) const;

    /**
     * Get recommended model based on total system RAM
     * (One-time decision when app starts)
     */
    ModelSize get_recommended_model_for_device();

    /**
     * Enable/disable automatic model switching
     */
    void set_auto_switching_enabled(bool enabled);

    bool is_auto_switching_enabled() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace llm
} // namespace kipepeo
