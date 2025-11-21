#pragma once

#include <cstddef>
#include <cstdint>

namespace kipepeo {
namespace llm {

// Model types
enum class ModelType {
    LLAMA_3_2_7B,
    LLAMA_3_2_34B,
    CUSTOM
};

// Quantization types
enum class QuantizationType {
    Q1_K_M,      // 1.58-bit quantization
    Q1_0,        // 1-bit quantization (AfricaQuant)
    Q4_0,
    Q8_0,
    F16,
    F32
};

// Inference parameters
struct InferenceParams {
    int32_t max_tokens = 512;
    float temperature = 0.7f;
    int32_t top_k = 40;
    float top_p = 0.9f;
    int32_t repeat_penalty = 1.1f;
    bool use_swahili_tokenizer = true;
};

// Performance metrics
struct PerformanceMetrics {
    float tokens_per_second = 0.0f;
    size_t memory_used_bytes = 0;
    float cpu_usage_percent = 0.0f;
};

} // namespace llm
} // namespace kipepeo

