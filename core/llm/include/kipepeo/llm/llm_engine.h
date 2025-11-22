#pragma once

#include <cstdint>
#include <algorithm>

namespace kipepeo {
namespace llm {

// Forward declarations
class ModelLoader;
class InferenceEngine;

/**
 * Main LLM Engine interface
 * Provides high-level API for offline LLM inference on Android devices
 */
class LLMEngine {
public:
    LLMEngine();
    ~LLMEngine();

    // Initialize engine with model path
    bool initialize(const char* model_path);
    
    // Initialize engine with model path and configuration options
    struct InitParams {
        uint32_t n_ctx = 2048;          // Context window size
        uint32_t n_batch = 512;         // Batch size for decoding
        uint32_t n_threads = 0;         // Number of threads (0 = auto-detect)
        uint32_t n_threads_batch = 0;   // Batch threads (0 = auto-detect)
        bool use_mmap = true;           // Memory-map the GGUF file
        bool use_mlock = false;         // Lock memory pages
    };
    bool initialize(const char* model_path, const InitParams& params);

    // Generation parameters
    struct GenerationParams {
        int max_tokens = 256;
        float temperature = 0.8f;       // Range: 0.0-2.0, validated
        int top_k = 40;                 // Range: 0-1000, validated
        float top_p = 0.9f;             // Range: 0.0-1.0, validated
        float repeat_penalty = 1.1f;    // Range: 1.0-2.0, validated
        
        // Validate parameters and clamp to valid ranges
        void validate() {
            temperature = std::max(0.0f, std::min(2.0f, temperature));
            top_k = std::max(0, std::min(1000, top_k));
            top_p = std::max(0.0f, std::min(1.0f, top_p));
            repeat_penalty = std::max(1.0f, std::min(2.0f, repeat_penalty));
            max_tokens = std::max(1, std::min(4096, max_tokens));
        }
    };

    // Generate text from prompt (simple version)
    bool generate(const char* prompt, char* output, size_t output_size);
    
    // Generate text with custom parameters
    bool generate(const char* prompt, char* output, size_t output_size, const GenerationParams& params);

    // Get inference speed (tokens per second)
    float get_tokens_per_second() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace llm
} // namespace kipepeo

