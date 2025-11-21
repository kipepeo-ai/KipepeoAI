#include "kipepeo/llm/model_switcher.h"
#include <fstream>
#include <sstream>
#include <map>
#include <cstring>

#ifdef __ANDROID__
#include <sys/sysinfo.h>
#endif

namespace kipepeo {
namespace llm {

// Implementation class
class ModelSwitcher::Impl {
public:
    std::map<ModelSize, ModelInfo> models_;
    bool auto_switching_enabled_ = true;

    ModelInfo* find_model(ModelSize size) {
        auto it = models_.find(size);
        return (it != models_.end()) ? &it->second : nullptr;
    }

    const ModelInfo* find_model(ModelSize size) const {
        auto it = models_.find(size);
        return (it != models_.end()) ? &it->second : nullptr;
    }
};

ModelSwitcher::ModelSwitcher() : impl_(new Impl()) {}

ModelSwitcher::~ModelSwitcher() {
    delete impl_;
}

bool ModelSwitcher::register_model(ModelSize size, const std::string& path,
                                   uint64_t required_ram_mb, uint64_t optimal_ram_mb) {
    ModelInfo info;
    info.size = size;
    info.model_path = path;
    info.required_ram_mb = required_ram_mb;
    info.optimal_ram_mb = optimal_ram_mb;
    info.is_loaded = false;

    impl_->models_[size] = info;
    return true;
}

SystemMemoryInfo ModelSwitcher::get_memory_info() {
    SystemMemoryInfo mem_info = {0};

#ifdef __ANDROID__
    // Read /proc/meminfo on Android/Linux
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        uint64_t mem_total = 0, mem_free = 0, mem_available = 0;
        uint64_t buffers = 0, cached = 0;

        while (std::getline(meminfo, line)) {
            std::istringstream iss(line);
            std::string key;
            uint64_t value;
            std::string unit;

            iss >> key >> value >> unit;

            if (key == "MemTotal:") {
                mem_total = value; // in KB
            } else if (key == "MemFree:") {
                mem_free = value;
            } else if (key == "MemAvailable:") {
                mem_available = value;
            } else if (key == "Buffers:") {
                buffers = value;
            } else if (key == "Cached:") {
                cached = value;
            }
        }

        // Convert KB to MB
        mem_info.total_ram_mb = mem_total / 1024;
        mem_info.free_ram_mb = mem_free / 1024;
        
        // MemAvailable is the best indicator if available
        if (mem_available > 0) {
            mem_info.available_ram_mb = mem_available / 1024;
        } else {
            // Fallback: estimate available = free + buffers + cached
            mem_info.available_ram_mb = (mem_free + buffers + cached) / 1024;
        }

        if (mem_total > 0) {
            mem_info.usage_percent = 100.0f * (1.0f - (float)mem_available / (float)mem_total);
        }

        meminfo.close();
    }
#else
    // Fallback for non-Android platforms (desktop testing)
    // Assume 8 GB total, 4 GB available
    mem_info.total_ram_mb = 8192;
    mem_info.available_ram_mb = 4096;
    mem_info.free_ram_mb = 3072;
    mem_info.usage_percent = 50.0f;
#endif

    return mem_info;
}

float ModelSwitcher::get_available_ram_gb() {
    SystemMemoryInfo info = get_memory_info();
    return info.available_ram_mb / 1024.0f;
}

ModelSize ModelSwitcher::select_best_model(uint64_t min_free_ram_mb) {
    SystemMemoryInfo mem_info = get_memory_info();
    uint64_t usable_ram = mem_info.available_ram_mb - min_free_ram_mb;

    // Try to select largest model that fits
    ModelSize candidates[] = {ModelSize::MODEL_70B, ModelSize::MODEL_34B,
                             ModelSize::MODEL_13B, ModelSize::MODEL_7B};

    for (ModelSize size : candidates) {
        const ModelInfo* info = impl_->find_model(size);
        if (info && info->required_ram_mb <= usable_ram) {
            return size;
        }
    }

    // Default to smallest if registered, otherwise unknown
    if (impl_->find_model(ModelSize::MODEL_7B)) {
        return ModelSize::MODEL_7B;
    }

    return ModelSize::MODEL_UNKNOWN;
}

bool ModelSwitcher::should_downgrade(ModelSize current_size, uint64_t min_free_ram_mb) {
    SystemMemoryInfo mem_info = get_memory_info();
    
    // Downgrade if free RAM is below minimum threshold
    if (mem_info.free_ram_mb < min_free_ram_mb) {
        return true;
    }

    // Downgrade if system is under heavy memory pressure (>90% used)
    if (mem_info.usage_percent > 90.0f) {
        return true;
    }

    return false;
}

bool ModelSwitcher::can_upgrade(ModelSize current_size, uint64_t min_free_ram_mb) {
    if (current_size == ModelSize::MODEL_70B) {
        return false; // Already at largest
    }

    SystemMemoryInfo mem_info = get_memory_info();
    uint64_t usable_ram = mem_info.available_ram_mb - min_free_ram_mb;

    // Check next size up
    ModelSize next_size;
    if (current_size == ModelSize::MODEL_7B) next_size = ModelSize::MODEL_13B;
    else if (current_size == ModelSize::MODEL_13B) next_size = ModelSize::MODEL_34B;
    else if (current_size == ModelSize::MODEL_34B) next_size = ModelSize::MODEL_70B;
    else return false;

    const ModelInfo* next_info = impl_->find_model(next_size);
    return next_info && next_info->required_ram_mb <= usable_ram;
}

const ModelInfo* ModelSwitcher::get_model_info(ModelSize size) const {
    return impl_->find_model(size);
}

ModelSize ModelSwitcher::get_recommended_model_for_device() {
    SystemMemoryInfo mem_info = get_memory_info();
    uint64_t total_ram = mem_info.total_ram_mb;

    // Recommended models based on total device RAM:
    // 16+ GB -> 70B
    // 12-15 GB -> 34B
    // 8-11 GB -> 13B
    // 6-7 GB -> 7B
    // < 6 GB -> 7B (may struggle)

    if (total_ram >= 16384) return ModelSize::MODEL_70B;
    else if (total_ram >= 12288) return ModelSize::MODEL_34B;
    else if (total_ram >= 8192) return ModelSize::MODEL_13B;
    else return ModelSize::MODEL_7B;
}

void ModelSwitcher::set_auto_switching_enabled(bool enabled) {
    impl_->auto_switching_enabled_ = enabled;
}

bool ModelSwitcher::is_auto_switching_enabled() const {
    return impl_->auto_switching_enabled_;
}

} // namespace llm
} // namespace kipepeo
