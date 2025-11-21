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

    // Generate text from prompt
    bool generate(const char* prompt, char* output, size_t output_size);

    // Get inference speed (tokens per second)
    float get_tokens_per_second() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace llm
} // namespace kipepeo

