#pragma once

#include "kipepeo/quantization/africa_quant.h"
#include <string>

// Forward declarations from llama.cpp
struct llama_model;
struct llama_context;

namespace kipepeo {
namespace llm {

/**
 * Integration layer for llama.cpp with AfricaQuant support
 * 
 * This wrapper extends llama.cpp to support:
 * - AfricaQuant 1.28-bit and 1.58-bit quantization formats
 * - NEON kernel injection for optimized inference
 * - Custom GGUF format extensions
 */

enum class QuantFormat {
    GGUF_Q4_0,           // Standard 4-bit from llama.cpp
    GGUF_Q4_1,
    GGUF_Q8_0,           // Standard 8-bit
    AFRICA_QUANT_1_28,   // AfricaQuant 1.28-bit
    AFRICA_QUANT_1_58    // AfricaQuant 1.58-bit
};

struct ModelLoadOptions {
    std::string model_path;
    QuantFormat quant_format;
    uint32_t n_ctx;              // Context size
    uint32_t n_batch;            // Batch size
    uint32_t n_threads;          // Number of threads
    bool use_mmap;
    bool use_neon_kernels;       // Enable NEON optimizations
};

class LlamaIntegration {
public:
    LlamaIntegration();
    ~LlamaIntegration();

    /**
     * Register AfricaQuant formats with llama.cpp
     * Must be called before loading models
     */
    static bool register_africa_quant_formats();

    /**
     * Load model with optional AfricaQuant quantization
     */
    bool load_model(const ModelLoadOptions& options);

    /**
     * Unload current model
     */
    void unload_model();

    /**
     * Get underlying llama model/context for advanced usage
     */
    llama_model* get_llama_model();
    llama_context* get_llama_context();

    /**
     * Check if model is loaded
     */
    bool is_loaded() const;

    /**
     * Get current quantization format
     */
    QuantFormat get_quant_format() const;

    /**
     * Convert standard GGUF model to AfricaQuant format
     * 
     * @param input_path Path to standard GGUF model
     * @param output_path Path for output AfricaQuant model
     * @param target_format Target quantization format
     * @return true on success
     */
    static bool convert_to_africa_quant(
        const std::string& input_path,
        const std::string& output_path,
        QuantFormat target_format
    );

private:
    class Impl;
    Impl* impl_;
};

} // namespace llm
} // namespace kipepeo
