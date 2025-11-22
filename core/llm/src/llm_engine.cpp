#include "kipepeo/llm/llm_engine.h"
#include "llama.h"
#include <cstring>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <algorithm>
#include <cstdint>

namespace kipepeo {
namespace llm {

class LLMEngine::Impl {
public:
    llama_model* model = nullptr;
    llama_context* ctx = nullptr;
    llama_batch batch;
    // Performance tracking
    int n_tokens_generated = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    float tokens_per_second = 0.0f;
    ~Impl() {
        if (ctx) {
            llama_free(ctx);
            ctx = nullptr;
        }
        if (model) {
            llama_model_free(model);
            model = nullptr;
        }
        llama_batch_free(batch);
    }
};

LLMEngine::LLMEngine() : impl_(new Impl()) {
    // Initialize llama backend (sets up threading, etc.)
    if (llama_backend_init() != 0) {
        // Backend initialization failed - this is a critical error
        // The engine will be in an invalid state
    }
    // Prepare a reusable batch with a reasonable size for mobile devices
    // Will be resized if needed during initialization
    impl_->batch = llama_batch_init(512, 0, 1);
}

LLMEngine::~LLMEngine() {
    delete impl_;
    // Cleanup llama backend
    llama_backend_free();
}

// Helper function to detect optimal thread count
static uint32_t detect_optimal_threads(uint32_t requested) {
    if (requested > 0) {
        return requested;
    }
    // Auto-detect: use hardware concurrency, but cap at reasonable limit
    unsigned int hw_threads = std::thread::hardware_concurrency();
    if (hw_threads == 0) {
        // Fallback if hardware_concurrency() returns 0
        hw_threads = 4;
    }
    // For mobile devices, use 75% of available cores for inference
    // This leaves resources for the OS and other apps
    return std::max(1u, (hw_threads * 3) / 4);
}

bool LLMEngine::initialize(const char* model_path) {
    InitParams default_params;
    return initialize(model_path, default_params);
}

bool LLMEngine::initialize(const char* model_path, const InitParams& params) {
    if (!model_path || std::strlen(model_path) == 0) {
        return false;
    }
    
    // Validate and compute thread counts
    uint32_t n_threads = detect_optimal_threads(params.n_threads);
    uint32_t n_threads_batch = params.n_threads_batch > 0 
        ? params.n_threads_batch 
        : detect_optimal_threads(0);
    
    // Model loading parameters – tuned for Android/ARM
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0; // No GPU on typical Android devices
    model_params.use_mmap = params.use_mmap; // Memory‑map the GGUF file for fast loading
    model_params.use_mlock = params.use_mlock;
    
    impl_->model = llama_model_load_from_file(model_path, model_params);
    if (!impl_->model) {
        return false;
    }
    
    // Context parameters – configurable for different device capabilities
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = params.n_ctx;      // Context window size
    ctx_params.n_batch = params.n_batch;  // Batch size for decoding
    ctx_params.n_threads = n_threads;
    ctx_params.n_threads_batch = n_threads_batch;
    
    // Resize batch if needed
    if (params.n_batch > 512) {
        llama_batch_free(impl_->batch);
        impl_->batch = llama_batch_init(params.n_batch, 0, 1);
    }
    
    impl_->ctx = llama_init_from_model(impl_->model, ctx_params);
    if (!impl_->ctx) {
        llama_model_free(impl_->model);
        impl_->model = nullptr;
        return false;
    }
    return true;
}

// Generation parameters allowing fine‑grained control over sampling
struct LLMEngine::GenerationParams {
    int max_tokens = 256;      // Upper bound on generated tokens
    float temperature = 0.8f;  // Controls randomness – 0 = deterministic
    int top_k = 40;            // Top‑k sampling
    float top_p = 0.9f;        // Top‑p (nucleus) sampling
    float repeat_penalty = 1.1f; // Penalty for repeating tokens
};

// Helper: build a llama sampler chain based on the provided parameters
static llama_sampler* create_sampler(const LLMEngine::GenerationParams& params) {
    llama_sampler_chain_params chain_params = llama_sampler_chain_default_params();
    llama_sampler* chain = llama_sampler_chain_init(&chain_params);
    if (!chain) {
        return nullptr;
    }
    
    // Temperature
    if (params.temperature != 1.0f) {
        llama_sampler* temp = llama_sampler_init_temperature(params.temperature);
        if (temp) {
            llama_sampler_chain_add(chain, temp);
        }
    }
    // Top‑k
    if (params.top_k > 0) {
        llama_sampler* topk = llama_sampler_init_top_k(params.top_k);
        if (topk) {
            llama_sampler_chain_add(chain, topk);
        }
    }
    // Top‑p
    if (params.top_p < 1.0f) {
        llama_sampler* topp = llama_sampler_init_top_p(params.top_p);
        if (topp) {
            llama_sampler_chain_add(chain, topp);
        }
    }
    // Repetition penalty
    if (params.repeat_penalty != 1.0f) {
        llama_sampler* rep = llama_sampler_init_repeat(params.repeat_penalty);
        if (rep) {
            llama_sampler_chain_add(chain, rep);
        }
    }
    return chain;
}

// Default generate – forwards to overload with default parameters
bool LLMEngine::generate(const char* prompt, char* output, size_t output_size) {
    GenerationParams default_params;
    return generate(prompt, output, output_size, default_params);
}

// Core generation implementation with advanced sampling
bool LLMEngine::generate(const char* prompt, char* output, size_t output_size, const GenerationParams& params) {
    // Validate inputs
    if (!impl_->ctx || !impl_->model || !prompt || !output || output_size == 0) {
        return false;
    }
    
    // Validate and clamp parameters
    GenerationParams validated_params = params;
    validated_params.validate();
    
    std::memset(output, 0, output_size);

    // Optimized single-pass tokenization - estimate size first, then tokenize
    const size_t prompt_len = std::strlen(prompt);
    if (prompt_len == 0) {
        return false;
    }
    
    // Estimate token count (rough: 1 token per 4 chars for most languages)
    size_t estimated_tokens = (prompt_len / 4) + 16; // Add buffer
    std::vector<llama_token> prompt_tokens;
    prompt_tokens.reserve(estimated_tokens);
    
    // Single-pass tokenization with dynamic buffer
    int n_tokens = llama_tokenize(impl_->model, prompt, prompt_len, nullptr, 0, true, false);
    if (n_tokens < 0) {
        // Negative means we need more space - resize and try again
        prompt_tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(impl_->model, prompt, prompt_len, prompt_tokens.data(), prompt_tokens.size(), true, false);
    } else {
        // Positive means we got the count, resize and tokenize
        prompt_tokens.resize(n_tokens);
        n_tokens = llama_tokenize(impl_->model, prompt, prompt_len, prompt_tokens.data(), prompt_tokens.size(), true, false);
    }
    
    if (n_tokens < 0 || prompt_tokens.empty()) {
        return false;
    }

    // Prepare batch for the prompt
    llama_batch_clear(impl_->batch);
    for (int i = 0; i < n_tokens; ++i) {
        llama_batch_add(impl_->batch, prompt_tokens[i], i, {0}, false);
    }
    impl_->batch.logits[impl_->batch.n_tokens - 1] = true;
    
    if (llama_decode(impl_->ctx, impl_->batch) != 0) {
        return false;
    }

    // Create sampler based on user‑provided parameters
    llama_sampler* sampler = create_sampler(validated_params);
    if (!sampler) {
        return false;
    }

    // Performance tracking start
    impl_->start_time = std::chrono::high_resolution_clock::now();
    impl_->n_tokens_generated = 0;

    // Pre-allocate string capacity for better performance
    // Estimate: average 4-8 bytes per token
    std::string generated;
    generated.reserve(validated_params.max_tokens * 8);
    
    int n_cur = impl_->batch.n_tokens;
    int generated_tokens = 0;
    
    while (generated_tokens < validated_params.max_tokens) {
        const float* logits = llama_get_logits_ith(impl_->ctx, impl_->batch.n_tokens - 1);
        if (!logits) {
            break;
        }
        
        llama_token new_token = llama_sampler_sample(sampler, impl_->ctx, logits);
        llama_sampler_accept(sampler, new_token);
        
        if (llama_token_is_eog(impl_->model, new_token)) {
            break;
        }
        
        // Convert token to string using dynamic buffer
        // First, get the required size
        int len = llama_token_to_piece(impl_->model, new_token, nullptr, 0, 0, false);
        if (len > 0) {
            // Allocate buffer of exact size needed
            std::vector<char> buf(len + 1);
            int actual_len = llama_token_to_piece(impl_->model, new_token, buf.data(), buf.size(), 0, false);
            if (actual_len > 0) {
                generated.append(buf.data(), actual_len);
            }
        }
        
        // Prepare next batch
        llama_batch_clear(impl_->batch);
        llama_batch_add(impl_->batch, new_token, n_cur, {0}, true);
        ++n_cur;
        ++generated_tokens;
        ++impl_->n_tokens_generated;
        
        if (llama_decode(impl_->ctx, impl_->batch) != 0) {
            break;
        }
    }
    
    // Cleanup sampler
    llama_sampler_free(sampler);

    // Compute tokens per second
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - impl_->start_time);
    if (dur.count() > 0) {
        impl_->tokens_per_second = (impl_->n_tokens_generated * 1000.0f) / dur.count();
    }

    // Copy to output buffer
    size_t copy_len = std::min(generated.size(), output_size - 1);
    std::memcpy(output, generated.c_str(), copy_len);
    output[copy_len] = '\0';
    return !generated.empty();
}

float LLMEngine::get_tokens_per_second() const {
    return impl_->tokens_per_second;
}

} // namespace llm
} // namespace kipepeo
