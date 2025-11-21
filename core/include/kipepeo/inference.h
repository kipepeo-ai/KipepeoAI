#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Kipepeo LLM Inference C API
 * 
 * Clean C interface for LLM inference operations with support for:
 * - Multiple quantization formats (GGUF standard + AfricaQuant 1.28/1.58-bit)
 * - Dynamic model switching based on available RAM
 * - Optimized for African mobile devices (Helio G99, Unisoc T606, Snapdragon 7s Gen 2)
 */

// Opaque handle types
typedef struct kipepeo_model kipepeo_model_t;
typedef struct kipepeo_context kipepeo_context_t;

// Error codes
typedef enum {
    KIPEPEO_SUCCESS = 0,
    KIPEPEO_ERROR_INVALID_PARAM = -1,
    KIPEPEO_ERROR_OUT_OF_MEMORY = -2,
    KIPEPEO_ERROR_MODEL_LOAD_FAILED = -3,
    KIPEPEO_ERROR_INFERENCE_FAILED = -4,
    KIPEPEO_ERROR_UNSUPPORTED_QUANT = -5,
} kipepeo_error_t;

// Quantization types
typedef enum {
    KIPEPEO_QUANT_F32 = 0,           // Full precision (fallback)
    KIPEPEO_QUANT_F16 = 1,           // Half precision
    KIPEPEO_QUANT_Q4_0 = 2,          // 4-bit quantization
    KIPEPEO_QUANT_Q4_1 = 3,          // 4-bit quantization variant
    KIPEPEO_QUANT_Q8_0 = 4,          // 8-bit quantization
    KIPEPEO_QUANT_AFRICA_1_28 = 100, // AfricaQuant 1.28-bit (Swahili/Sheng optimized)
    KIPEPEO_QUANT_AFRICA_1_58 = 101, // AfricaQuant 1.58-bit (better than BitNet)
} kipepeo_quant_type_t;

// Model size categories for dynamic switching
typedef enum {
    KIPEPEO_MODEL_7B = 0,
    KIPEPEO_MODEL_13B = 1,
    KIPEPEO_MODEL_34B = 2,
    KIPEPEO_MODEL_70B = 3,
} kipepeo_model_size_t;

// Model load parameters
typedef struct {
    const char* model_path;              // Path to GGUF model file
    kipepeo_quant_type_t quant_type;     // Quantization type (can be detected from file)
    uint32_t n_ctx;                      // Context size (default: 2048)
    uint32_t n_batch;                    // Batch size for prompt processing (default: 512)
    uint32_t n_threads;                  // Number of threads (default: auto-detect)
    bool use_mmap;                       // Use memory mapping (default: true)
    bool use_mlock;                      // Lock memory to prevent swapping (default: false)
    bool low_vram;                       // Low VRAM/RAM mode (default: auto-detect)
    bool enable_dynamic_switching;       // Enable dynamic model switching (default: true)
    float min_free_ram_gb;               // Minimum free RAM in GB before switching down (default: 1.0)
} kipepeo_model_params_t;

// Inference parameters
typedef struct {
    int32_t n_predict;        // Number of tokens to generate (-1 = infinite)
    int32_t top_k;            // Top-K sampling (default: 40)
    float top_p;              // Top-P (nucleus) sampling (default: 0.95)
    float temperature;        // Temperature (default: 0.8)
    float repeat_penalty;     // Repetition penalty (default: 1.1)
    uint32_t seed;            // Random seed (default: random)
    const char* stop_str;     // Stop generation on this string (optional)
} kipepeo_infer_params_t;

// Callback for streaming token generation
typedef void (*kipepeo_token_callback_t)(const char* token, void* user_data);

/**
 * Initialize Kipepeo inference library
 * Must be called once before using any other functions
 */
kipepeo_error_t kipepeo_init(void);

/**
 * Cleanup Kipepeo inference library
 * Should be called when done using the library
 */
void kipepeo_cleanup(void);

/**
 * Get default model parameters
 */
kipepeo_model_params_t kipepeo_model_params_default(void);

/**
 * Get default inference parameters
 */
kipepeo_infer_params_t kipepeo_infer_params_default(void);

/**
 * Load a model from disk
 * 
 * @param params Model loading parameters
 * @param model Output model handle
 * @return Error code
 */
kipepeo_error_t kipepeo_model_load(
    const kipepeo_model_params_t* params,
    kipepeo_model_t** model
);

/**
 * Unload a model and free resources
 */
void kipepeo_model_free(kipepeo_model_t* model);

/**
 * Get model size category
 */
kipepeo_model_size_t kipepeo_model_get_size(const kipepeo_model_t* model);

/**
 * Get current quantization type
 */
kipepeo_quant_type_t kipepeo_model_get_quant_type(const kipepeo_model_t* model);

/**
 * Get available system RAM in GB
 */
float kipepeo_get_available_ram_gb(void);

/**
 * Create inference context from model
 * 
 * @param model Model handle
 * @param context Output context handle
 * @return Error code
 */
kipepeo_error_t kipepeo_context_create(
    kipepeo_model_t* model,
    kipepeo_context_t** context
);

/**
 * Free inference context
 */
void kipepeo_context_free(kipepeo_context_t* context);

/**
 * Reset context (clear conversation history)
 */
void kipepeo_context_reset(kipepeo_context_t* context);

/**
 * Generate tokens synchronously (blocking)
 * 
 * @param context Inference context
 * @param prompt Input prompt
 * @param params Inference parameters
 * @param output Buffer for generated text (allocated by caller)
 * @param output_size Size of output buffer
 * @return Error code
 */
kipepeo_error_t kipepeo_generate(
    kipepeo_context_t* context,
    const char* prompt,
    const kipepeo_infer_params_t* params,
    char* output,
    size_t output_size
);

/**
 * Generate tokens with streaming callback (asynchronous-style)
 * 
 * @param context Inference context
 * @param prompt Input prompt
 * @param params Inference parameters
 * @param callback Token callback (called for each generated token)
 * @param user_data User data passed to callback
 * @return Error code
 */
kipepeo_error_t kipepeo_generate_streaming(
    kipepeo_context_t* context,
    const char* prompt,
    const kipepeo_infer_params_t* params,
    kipepeo_token_callback_t callback,
    void* user_data
);

/**
 * Manually trigger model switching based on current RAM
 * (Normally happens automatically when dynamic switching is enabled)
 * 
 * @param context Inference context
 * @param target_size Target model size (or auto-detect if KIPEPEO_MODEL_7B with available_ram < threshold)
 * @return Error code
 */
kipepeo_error_t kipepeo_switch_model(
    kipepeo_context_t* context,
    kipepeo_model_size_t target_size
);

/**
 * Get last error message
 */
const char* kipepeo_get_error_string(kipepeo_error_t error);

/**
 * Get library version string
 */
const char* kipepeo_get_version(void);

/**
 * Check if NEON optimizations are enabled
 */
bool kipepeo_has_neon_support(void);

/**
 * Get detected SoC type (for debugging/logging)
 * Returns: "Helio G99", "Unisoc T606", "Snapdragon 7s Gen 2", "Generic ARM", etc.
 */
const char* kipepeo_get_soc_type(void);

#ifdef __cplusplus
}
#endif
