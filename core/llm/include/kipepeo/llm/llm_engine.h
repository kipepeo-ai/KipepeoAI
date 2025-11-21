#pragma once

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

    // Generation parameters
    struct GenerationParams {
        int max_tokens = 256;
        float temperature = 0.8f;
        int top_k = 40;
        float top_p = 0.9f;
        float repeat_penalty = 1.1f;
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

