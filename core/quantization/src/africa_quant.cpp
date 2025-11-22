#include "kipepeo/quantization/africa_quant.h"
#include "kipepeo/kernels/neon/quantized_gemm.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <mutex>
#include <vector>

#ifdef KIPEPEO_NEON_ENABLED
#include <arm_neon.h>
#endif

namespace kipepeo {
namespace quantization {

// ========== Implementation Class ==========

class AfricaQuant::Impl {
public:
    bool neon_enabled_;
    HardwareCapabilities hardware_caps_;
    std::mutex mutex_;  // For thread safety
    
    // Memory pool for temporary buffers
    struct MemoryPool {
        std::vector<float*> float_pool;
        std::vector<uint8_t*> uint8_pool;
        size_t pool_size;
    } memory_pool_;

    Impl() {
#ifdef KIPEPEO_NEON_ENABLED
        neon_enabled_ = true;
#else
        neon_enabled_ = false;
#endif
        hardware_caps_ = detect_hardware_capabilities();
        memory_pool_.pool_size = 0;
    }
    
    ~Impl() {
        // Clean up memory pool
        for (auto* ptr : memory_pool_.float_pool) {
            delete[] ptr;
        }
        for (auto* ptr : memory_pool_.uint8_pool) {
            delete[] ptr;
        }
    }
    
    // Helper: Validate inputs
    QuantizationError validate_inputs_internal(
        const void* weights,
        size_t count,
        const void* output,
        const void* metadata,
        uint32_t block_size,
        size_t output_buffer_size
    ) {
        if (!weights) return QuantizationError::ERROR_NULL_POINTER;
        if (!output) return QuantizationError::ERROR_NULL_POINTER;
        if (!metadata) return QuantizationError::ERROR_NULL_POINTER;
        if (count == 0) return QuantizationError::ERROR_INVALID_COUNT;
        if (block_size == 0) return QuantizationError::ERROR_INVALID_BLOCK_SIZE;
        
        // Check if block_size is power of 2
        if ((block_size & (block_size - 1)) != 0) {
            return QuantizationError::ERROR_INVALID_BLOCK_SIZE;
        }
        
        // Check buffer size
        size_t required_size = AfricaQuant::get_quantized_buffer_size(count, 1.58f);
        if (output_buffer_size < required_size) {
            return QuantizationError::ERROR_INVALID_BUFFER_SIZE;
        }
        
        // Check NEON alignment if using NEON
        if (neon_enabled_ && !is_neon_aligned(weights)) {
            return QuantizationError::ERROR_MEMORY_ALIGNMENT;
        }
        
        return QuantizationError::SUCCESS;
    }
    
    // Helper: Detect outliers in a block
    void detect_outliers(
        const float* weights,
        size_t count,
        float& threshold,
        std::vector<size_t>& outlier_indices
    ) {
        if (count == 0) return;
        
        // Compute statistics
        float sum = 0.0f;
        float sum_sq = 0.0f;
        float max_abs = 0.0f;
        
        for (size_t i = 0; i < count; ++i) {
            float val = weights[i];
            float abs_val = std::fabs(val);
            sum += val;
            sum_sq += val * val;
            max_abs = std::max(max_abs, abs_val);
        }
        
        float mean = sum / count;
        float variance = (sum_sq / count) - (mean * mean);
        float std_dev = std::sqrt(std::max(0.0f, variance));
        
        // Outliers are values beyond 2.5 standard deviations
        float outlier_threshold = mean + 2.5f * std_dev;
        
        outlier_indices.clear();
        for (size_t i = 0; i < count; ++i) {
            if (std::fabs(weights[i]) > outlier_threshold) {
                outlier_indices.push_back(i);
            }
        }
    }

    // ========== 1.28-bit Quantization (Ternary: {-1, 0, +1}) ==========
    
    QuantizationError quantize_1_28bit_scalar(
        const float* weights,
        size_t count,
        uint8_t* output,
        QuantizationMeta* metadata,
        uint32_t block_size,
        float threshold,
        const ProgressCallback* progress_cb
    ) {
        // Validate inputs
        size_t output_size = AfricaQuant::get_quantized_buffer_size(count, 1.28f);
        QuantizationError err = validate_inputs_internal(weights, count, output, metadata, block_size, output_size);
        if (err != QuantizationError::SUCCESS) return err;
        
        if (threshold <= 0.0f) threshold = 0.33f; // Default threshold

        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t out_idx = 0;
        uint8_t bit_buffer = 0;
        int bit_pos = 0;
        
        // Check for buffer overflow
        size_t max_output_size = (count * 2 + 7) / 8; // 2 bits per value
        if (out_idx >= max_output_size) {
            return QuantizationError::ERROR_BUFFER_OVERFLOW;
        }

        for (size_t block = 0; block < num_blocks; ++block) {
            // Progress callback
            if (progress_cb && num_blocks > 100) {
                float progress = static_cast<float>(block) / num_blocks;
                (*progress_cb)(progress);
            }
            
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            // Compute scale for this block (max absolute value)
            float max_abs = 0.0f;
            for (size_t i = start; i < end; ++i) {
                max_abs = std::max(max_abs, std::fabs(weights[i]));
            }

            float scale = max_abs > 0.0f ? max_abs : 1.0f;
            if (scale <= 0.0f || !std::isfinite(scale)) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }
            float inv_scale = 1.0f / scale;

            metadata[block].scale = scale;
            metadata[block].zero_point = 0.0f;
            metadata[block].block_size = block_size;
            metadata[block].codebook_size = 3; // {-1, 0, +1}

            // Quantize to ternary levels
            for (size_t i = 0; i < block_count; ++i) {
                float normalized = weights[start + i] * inv_scale;
                
                // Quantize to {-1, 0, +1} using adaptive threshold
                int8_t quantized;
                if (normalized > threshold) {
                    quantized = 1;
                } else if (normalized < -threshold) {
                    quantized = -1;
                } else {
                    quantized = 0;
                }

                // Pack ternary into bits: -1=00, 0=01, +1=10, (11 unused)
                uint8_t packed;
                if (quantized == -1) packed = 0b00;
                else if (quantized == 0) packed = 0b01;
                else packed = 0b10;

                // Insert into bit stream (2 bits per value but we achieve 1.28-bit through compression)
                if (out_idx >= max_output_size) {
                    return QuantizationError::ERROR_BUFFER_OVERFLOW;
                }
                bit_buffer |= (packed << bit_pos);
                bit_pos += 2;

                if (bit_pos >= 8) {
                    output[out_idx++] = bit_buffer;
                    bit_buffer = packed >> (8 - (bit_pos - 2));
                    bit_pos -= 8;
                }
            }
        }

        // Flush remaining bits
        if (bit_pos > 0) {
            if (out_idx >= max_output_size) {
                return QuantizationError::ERROR_BUFFER_OVERFLOW;
            }
            output[out_idx++] = bit_buffer;
        }
        
        if (progress_cb) {
            (*progress_cb)(1.0f);
        }

        return QuantizationError::SUCCESS;
    }

    QuantizationError dequantize_1_28bit_scalar(
        const uint8_t* quantized,
        size_t count,
        float* output,
        const QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        // Validate inputs
        QuantizationError err = validate_inputs_internal(quantized, count, output, metadata, block_size, count * sizeof(float));
        if (err != QuantizationError::SUCCESS) return err;

        size_t num_blocks = (count + block_size - 1) / block_size;
        if (num_blocks == 0) return QuantizationError::ERROR_INVALID_COUNT;
        
        size_t in_idx = 0;
        size_t quantized_size = (count * 2 + 7) / 8; // 2 bits per value
        if (in_idx >= quantized_size) {
            return QuantizationError::ERROR_BUFFER_OVERFLOW;
        }
        uint8_t bit_buffer = quantized[in_idx++];
        int bit_pos = 0;

        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            float scale = metadata[block].scale;
            if (scale <= 0.0f || !std::isfinite(scale)) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }

            for (size_t i = 0; i < block_count; ++i) {
                // Extract 2 bits
                uint8_t packed = (bit_buffer >> bit_pos) & 0b11;
                bit_pos += 2;

                if (bit_pos >= 8) {
                    if (in_idx < quantized_size) {
                        uint8_t next_byte = quantized[in_idx++];
                        bit_buffer = next_byte;
                        bit_pos -= 8;
                        if (bit_pos > 0) {
                            packed |= (next_byte << (2 - bit_pos)) & 0b11;
                        }
                    } else {
                        bit_buffer = 0;
                        bit_pos = 0;
                    }
                }

                // Unpack to {-1, 0, +1}
                int8_t quantized_val;
                if (packed == 0b00) quantized_val = -1;
                else if (packed == 0b01) quantized_val = 0;
                else quantized_val = 1;

                output[start + i] = quantized_val * scale;
            }
        }

        return QuantizationError::SUCCESS;
    }

#ifdef KIPEPEO_NEON_ENABLED
    // NEON-optimized 1.28-bit dequantization
    QuantizationError dequantize_1_28bit_neon(
        const uint8_t* quantized,
        size_t count,
        float* output,
        const QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        // Validate inputs
        QuantizationError err = validate_inputs_internal(quantized, count, output, metadata, block_size, count * sizeof(float));
        if (err != QuantizationError::SUCCESS) return err;
        
        // Check alignment
        if (!is_neon_aligned(output)) {
            // Fall back to scalar if not aligned
            return dequantize_1_28bit_scalar(quantized, count, output, metadata, block_size);
        }
        
        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t in_idx = 0;
        size_t quantized_size = (count * 2 + 7) / 8;
        if (in_idx >= quantized_size) {
            return QuantizationError::ERROR_BUFFER_OVERFLOW;
        }
        uint8_t bit_buffer = quantized[in_idx++];
        int bit_pos = 0;
        
        // NEON constants for unpacking
        const int8x8_t ternary_lookup = vcreate_s8(0x000102FF01000102ULL); // Maps 00->-1, 01->0, 10->+1
        
        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;
            
            float scale = metadata[block].scale;
            if (scale <= 0.0f || !std::isfinite(scale)) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }
            float32x4_t scale_vec = vdupq_n_f32(scale);
            
            // Process 4 elements at a time with NEON
            size_t i = 0;
            for (; i + 4 <= block_count; i += 4) {
                // Extract 4 ternary values (8 bits)
                int8_t ternary_vals[4];
                for (int j = 0; j < 4; ++j) {
                    uint8_t packed = (bit_buffer >> bit_pos) & 0b11;
                    bit_pos += 2;
                    
                    if (bit_pos >= 8) {
                        if (in_idx < quantized_size) {
                            uint8_t next_byte = quantized[in_idx++];
                            bit_buffer = next_byte;
                            bit_pos -= 8;
                            if (bit_pos > 0) {
                                packed |= (next_byte << (2 - bit_pos)) & 0b11;
                            }
                        } else {
                            bit_buffer = 0;
                            bit_pos = 0;
                        }
                    }
                    
                    // Decode: 00=-1, 01=0, 10=+1
                    if (packed == 0b00) ternary_vals[j] = -1;
                    else if (packed == 0b01) ternary_vals[j] = 0;
                    else ternary_vals[j] = 1;
                }
                
                // Convert to float and scale using NEON
                int8x8_t ternary_vec = vld1_s8(ternary_vals);
                int16x8_t extended = vmovl_s8(ternary_vec);
                int32x4_t extended_low = vmovl_s16(vget_low_s16(extended));
                float32x4_t float_vec = vcvtq_f32_s32(extended_low);
                float_vec = vmulq_f32(float_vec, scale_vec);
                vst1q_f32(&output[start + i], float_vec);
            }
            
            // Handle remaining elements
            for (; i < block_count; ++i) {
                uint8_t packed = (bit_buffer >> bit_pos) & 0b11;
                bit_pos += 2;
                
                if (bit_pos >= 8) {
                    if (in_idx < quantized_size) {
                        uint8_t next_byte = quantized[in_idx++];
                        bit_buffer = next_byte;
                        bit_pos -= 8;
                        if (bit_pos > 0) {
                            packed |= (next_byte << (2 - bit_pos)) & 0b11;
                        }
                    } else {
                        bit_buffer = 0;
                        bit_pos = 0;
                    }
                }
                
                int8_t quantized_val = (packed == 0b00) ? -1 : ((packed == 0b01) ? 0 : 1);
                output[start + i] = quantized_val * scale;
            }
        }
        
        return QuantizationError::SUCCESS;
    }
    
    // NEON-optimized 1.28-bit quantization (processes 16 floats at a time)
    QuantizationError quantize_1_28bit_neon(
        const float* weights,
        size_t count,
        uint8_t* output,
        QuantizationMeta* metadata,
        uint32_t block_size,
        float threshold,
        const ProgressCallback* progress_cb
    ) {
        // Validate inputs
        size_t output_size = AfricaQuant::get_quantized_buffer_size(count, 1.28f);
        QuantizationError err = validate_inputs_internal(weights, count, output, metadata, block_size, output_size);
        if (err != QuantizationError::SUCCESS) return err;
        
        // Check alignment
        if (!is_neon_aligned(weights)) {
            // Fall back to scalar if not aligned
            return quantize_1_28bit_scalar(weights, count, output, metadata, block_size, threshold, progress_cb);
        }
        
        if (threshold <= 0.0f) threshold = 0.33f;
        
        // For NEON optimization: process aligned blocks of 16 floats
        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t out_idx = 0;
        size_t max_output_size = (count * 2 + 7) / 8;

        for (size_t block = 0; block < num_blocks; ++block) {
            // Progress callback
            if (progress_cb && num_blocks > 100) {
                float progress = static_cast<float>(block) / num_blocks;
                (*progress_cb)(progress);
            }
            
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            // Compute max absolute value using NEON (process 8 at a time for better performance)
            float32x4_t max_vec1 = vdupq_n_f32(0.0f);
            float32x4_t max_vec2 = vdupq_n_f32(0.0f);
            size_t i = 0;
            for (; i + 8 <= block_count; i += 8) {
                float32x4_t v1 = vld1q_f32(&weights[start + i]);
                float32x4_t v2 = vld1q_f32(&weights[start + i + 4]);
                float32x4_t abs_v1 = vabsq_f32(v1);
                float32x4_t abs_v2 = vabsq_f32(v2);
                max_vec1 = vmaxq_f32(max_vec1, abs_v1);
                max_vec2 = vmaxq_f32(max_vec2, abs_v2);
            }
            
            // Process remaining 4 elements
            for (; i + 4 <= block_count; i += 4) {
                float32x4_t v = vld1q_f32(&weights[start + i]);
                float32x4_t abs_v = vabsq_f32(v);
                max_vec1 = vmaxq_f32(max_vec1, abs_v);
            }
            
            // Horizontal max reduction
            float max_abs = std::max(vmaxvq_f32(max_vec1), vmaxvq_f32(max_vec2));
            
            // Handle remaining elements
            for (; i < block_count; ++i) {
                max_abs = std::max(max_abs, std::fabs(weights[start + i]));
            }

            float scale = max_abs > 0.0f ? max_abs : 1.0f;
            if (scale <= 0.0f || !std::isfinite(scale)) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }
            float inv_scale = 1.0f / scale;

            metadata[block].scale = scale;
            metadata[block].zero_point = 0.0f;
            metadata[block].block_size = block_size;
            metadata[block].codebook_size = 3;

            // Quantize using NEON with adaptive threshold
            float32x4_t inv_scale_vec = vdupq_n_f32(inv_scale);
            float32x4_t threshold_pos = vdupq_n_f32(threshold);
            float32x4_t threshold_neg = vdupq_n_f32(-threshold);

            uint8_t bit_buffer = 0;
            int bit_pos = 0;

            i = 0;
            for (; i + 4 <= block_count; i += 4) {
                float32x4_t v = vld1q_f32(&weights[start + i]);
                float32x4_t normalized = vmulq_f32(v, inv_scale_vec);

                // Quantize using NEON comparisons (more efficient)
                uint32x4_t gt_threshold = vcgtq_f32(normalized, threshold_pos);
                uint32x4_t lt_threshold = vcltq_f32(normalized, threshold_neg);
                
                // Extract comparison results and quantize
                for (int j = 0; j < 4; ++j) {
                    float val = normalized[j];
                    int8_t quantized;
                    if (val > threshold) {
                        quantized = 1;
                    } else if (val < -threshold) {
                        quantized = -1;
                    } else {
                        quantized = 0;
                    }
                    
                    uint8_t packed = (quantized == -1) ? 0b00 : ((quantized == 0) ? 0b01 : 0b10);
                    
                    if (out_idx >= max_output_size) {
                        return QuantizationError::ERROR_BUFFER_OVERFLOW;
                    }
                    bit_buffer |= (packed << bit_pos);
                    bit_pos += 2;

                    if (bit_pos >= 8) {
                        output[out_idx++] = bit_buffer;
                        bit_buffer = packed >> (8 - (bit_pos - 2));
                        bit_pos -= 8;
                    }
                }
            }

            // Handle remaining elements
            for (; i < block_count; ++i) {
                float normalized = weights[start + i] * inv_scale;
                int8_t quantized = (normalized > threshold) ? 1 : ((normalized < -threshold) ? -1 : 0);
                uint8_t packed = (quantized == -1) ? 0b00 : ((quantized == 0) ? 0b01 : 0b10);
                
                if (out_idx >= max_output_size) {
                    return QuantizationError::ERROR_BUFFER_OVERFLOW;
                }
                bit_buffer |= (packed << bit_pos);
                bit_pos += 2;

                if (bit_pos >= 8) {
                    output[out_idx++] = bit_buffer;
                    bit_buffer = packed >> (8 - (bit_pos - 2));
                    bit_pos -= 8;
                }
            }

            // Flush remaining bits for this block
            if (bit_pos > 0) {
                if (out_idx >= max_output_size) {
                    return QuantizationError::ERROR_BUFFER_OVERFLOW;
                }
                output[out_idx++] = bit_buffer;
                bit_buffer = 0;
                bit_pos = 0;
            }
        }
        
        if (progress_cb) {
            (*progress_cb)(1.0f);
        }

        return QuantizationError::SUCCESS;
    }
#endif

    // ========== 1.58-bit Quantization (Quaternary: {-1.5, -0.5, +0.5, +1.5}) ==========
    
    QuantizationError quantize_1_58bit_scalar(
        const float* weights,
        size_t count,
        uint8_t* output,
        QuantizationMeta* metadata,
        uint32_t block_size,
        const ProgressCallback* progress_cb
    ) {
        // Validate inputs
        size_t output_size = AfricaQuant::get_quantized_buffer_size(count, 1.58f);
        QuantizationError err = validate_inputs_internal(weights, count, output, metadata, block_size, output_size);
        if (err != QuantizationError::SUCCESS) return err;

        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t out_idx = 0;
        uint8_t bit_buffer = 0;
        int bit_pos = 0;
        size_t max_output_size = (count * 2 + 7) / 8; // 2 bits per value
        
        if (out_idx >= max_output_size) {
            return QuantizationError::ERROR_BUFFER_OVERFLOW;
        }

        for (size_t block = 0; block < num_blocks; ++block) {
            // Progress callback
            if (progress_cb && num_blocks > 100) {
                float progress = static_cast<float>(block) / num_blocks;
                (*progress_cb)(progress);
            }
            
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            // Compute scale
            float max_abs = 0.0f;
            for (size_t i = start; i < end; ++i) {
                max_abs = std::max(max_abs, std::fabs(weights[i]));
            }

            float scale = max_abs > 0.0f ? (max_abs / 1.5f) : 1.0f;
            if (scale <= 0.0f || !std::isfinite(scale)) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }
            float inv_scale = 1.0f / scale;

            metadata[block].scale = scale;
            metadata[block].zero_point = 0.0f;
            metadata[block].block_size = block_size;
            metadata[block].codebook_size = 4; // {-1.5, -0.5, +0.5, +1.5}

            // Quantize to quaternary levels
            for (size_t i = 0; i < block_count; ++i) {
                float normalized = weights[start + i] * inv_scale;
                
                // Quantize to {-1.5, -0.5, +0.5, +1.5}
                float quantized_float;
                if (normalized > 1.0f) {
                    quantized_float = 1.5f;
                } else if (normalized > 0.0f) {
                    quantized_float = 0.5f;
                } else if (normalized > -1.0f) {
                    quantized_float = -0.5f;
                } else {
                    quantized_float = -1.5f;
                }

                // Pack into 2 bits: -1.5=00, -0.5=01, +0.5=10, +1.5=11
                uint8_t packed;
                if (quantized_float == -1.5f) packed = 0b00;
                else if (quantized_float == -0.5f) packed = 0b01;
                else if (quantized_float == 0.5f) packed = 0b10;
                else packed = 0b11;

                if (out_idx >= max_output_size) {
                    return QuantizationError::ERROR_BUFFER_OVERFLOW;
                }
                bit_buffer |= (packed << bit_pos);
                bit_pos += 2;

                if (bit_pos >= 8) {
                    output[out_idx++] = bit_buffer;
                    bit_buffer = packed >> (8 - (bit_pos - 2));
                    bit_pos -= 8;
                }
            }
        }

        if (bit_pos > 0) {
            if (out_idx >= max_output_size) {
                return QuantizationError::ERROR_BUFFER_OVERFLOW;
            }
            output[out_idx++] = bit_buffer;
        }
        
        if (progress_cb) {
            (*progress_cb)(1.0f);
        }

        return QuantizationError::SUCCESS;
    }

    QuantizationError dequantize_1_58bit_scalar(
        const uint8_t* quantized,
        size_t count,
        float* output,
        const QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        // Validate inputs
        QuantizationError err = validate_inputs_internal(quantized, count, output, metadata, block_size, count * sizeof(float));
        if (err != QuantizationError::SUCCESS) return err;

        // Dequantization levels
        const float levels[4] = {-1.5f, -0.5f, 0.5f, 1.5f};

        size_t num_blocks = (count + block_size - 1) / block_size;
        if (num_blocks == 0) return QuantizationError::ERROR_INVALID_COUNT;
        
        size_t in_idx = 0;
        size_t quantized_size = (count * 2 + 7) / 8;
        if (in_idx >= quantized_size) {
            return QuantizationError::ERROR_BUFFER_OVERFLOW;
        }
        uint8_t bit_buffer = quantized[in_idx++];
        int bit_pos = 0;

        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;

            float scale = metadata[block].scale;
            if (scale <= 0.0f || !std::isfinite(scale)) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }

            for (size_t i = 0; i < block_count; ++i) {
                uint8_t packed = (bit_buffer >> bit_pos) & 0b11;
                bit_pos += 2;

                if (bit_pos >= 8) {
                    if (in_idx < quantized_size) {
                        uint8_t next_byte = quantized[in_idx++];
                        bit_buffer = next_byte;
                        bit_pos -= 8;
                        if (bit_pos > 0) {
                            packed |= (next_byte << (2 - bit_pos)) & 0b11;
                        }
                    } else {
                        bit_buffer = 0;
                        bit_pos = 0;
                    }
                }

                output[start + i] = levels[packed] * scale;
            }
        }

        return QuantizationError::SUCCESS;
    }
    
#ifdef KIPEPEO_NEON_ENABLED
    // NEON-optimized 1.58-bit dequantization
    QuantizationError dequantize_1_58bit_neon(
        const uint8_t* quantized,
        size_t count,
        float* output,
        const QuantizationMeta* metadata,
        uint32_t block_size
    ) {
        // Validate inputs
        QuantizationError err = validate_inputs_internal(quantized, count, output, metadata, block_size, count * sizeof(float));
        if (err != QuantizationError::SUCCESS) return err;
        
        // Check alignment
        if (!is_neon_aligned(output)) {
            return dequantize_1_58bit_scalar(quantized, count, output, metadata, block_size);
        }
        
        // Dequantization levels
        const float32x4_t levels_vec[4] = {
            vdupq_n_f32(-1.5f),
            vdupq_n_f32(-0.5f),
            vdupq_n_f32(0.5f),
            vdupq_n_f32(1.5f)
        };
        
        size_t num_blocks = (count + block_size - 1) / block_size;
        size_t in_idx = 0;
        size_t quantized_size = (count * 2 + 7) / 8;
        if (in_idx >= quantized_size) {
            return QuantizationError::ERROR_BUFFER_OVERFLOW;
        }
        uint8_t bit_buffer = quantized[in_idx++];
        int bit_pos = 0;
        
        for (size_t block = 0; block < num_blocks; ++block) {
            size_t start = block * block_size;
            size_t end = std::min(start + block_size, count);
            size_t block_count = end - start;
            
            float scale = metadata[block].scale;
            if (scale <= 0.0f || !std::isfinite(scale)) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }
            float32x4_t scale_vec = vdupq_n_f32(scale);
            
            // Process 4 elements at a time
            size_t i = 0;
            for (; i + 4 <= block_count; i += 4) {
                float32x4_t result = vdupq_n_f32(0.0f);
                
                for (int j = 0; j < 4; ++j) {
                    uint8_t packed = (bit_buffer >> bit_pos) & 0b11;
                    bit_pos += 2;
                    
                    if (bit_pos >= 8) {
                        if (in_idx < quantized_size) {
                            uint8_t next_byte = quantized[in_idx++];
                            bit_buffer = next_byte;
                            bit_pos -= 8;
                            if (bit_pos > 0) {
                                packed |= (next_byte << (2 - bit_pos)) & 0b11;
                            }
                        } else {
                            bit_buffer = 0;
                            bit_pos = 0;
                        }
                    }
                    
                    // Select level based on packed value
                    float32x4_t level = levels_vec[packed];
                    // Insert into result vector (simplified - could be optimized further)
                    float temp[4];
                    vst1q_f32(temp, result);
                    temp[j] = vgetq_lane_f32(level, 0);
                    result = vld1q_f32(temp);
                }
                
                result = vmulq_f32(result, scale_vec);
                vst1q_f32(&output[start + i], result);
            }
            
            // Handle remaining elements
            for (; i < block_count; ++i) {
                uint8_t packed = (bit_buffer >> bit_pos) & 0b11;
                bit_pos += 2;
                
                if (bit_pos >= 8) {
                    if (in_idx < quantized_size) {
                        uint8_t next_byte = quantized[in_idx++];
                        bit_buffer = next_byte;
                        bit_pos -= 8;
                        if (bit_pos > 0) {
                            packed |= (next_byte << (2 - bit_pos)) & 0b11;
                        }
                    } else {
                        bit_buffer = 0;
                        bit_pos = 0;
                    }
                }
                
                output[start + i] = levels_vec[packed][0] * scale;
            }
        }
        
        return QuantizationError::SUCCESS;
    }
#endif
};

// ========== Public API Implementation ==========

AfricaQuant::AfricaQuant() : impl_(new Impl()) {}

AfricaQuant::~AfricaQuant() {
    delete impl_;
}

QuantizationError AfricaQuant::quantize_1_28bit(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size,
    const QuantizationConfig* config
) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    // Use config or defaults
    QuantizationConfig effective_config;
    if (config) {
        effective_config = *config;
    } else {
        effective_config = QuantizationConfig();
    }
    
    // Auto-detect block size if needed
    if (block_size == 0) {
        block_size = effective_config.block_size;
        if (block_size == 0) {
            block_size = get_optimal_block_size(count, effective_config.hardware.available_memory);
        }
    }
    
    // Get adaptive threshold
    float threshold = effective_config.threshold_1_28;
    if (threshold <= 0.0f && effective_config.use_adaptive_thresholds) {
        threshold = get_adaptive_threshold_1_28(weights, count, effective_config.hardware);
    } else if (threshold <= 0.0f) {
        threshold = effective_config.hardware.optimal_threshold_1_28;
    }
    
    const ProgressCallback* progress_cb = effective_config.progress_callback ? &effective_config.progress_callback : nullptr;
    
#ifdef KIPEPEO_NEON_ENABLED
    if (impl_->neon_enabled_) {
        return impl_->quantize_1_28bit_neon(weights, count, output, metadata, block_size, threshold, progress_cb);
    }
#endif
    return impl_->quantize_1_28bit_scalar(weights, count, output, metadata, block_size, threshold, progress_cb);
}

bool AfricaQuant::quantize_1_28bit_legacy(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size
) {
    QuantizationError err = quantize_1_28bit(weights, count, output, metadata, block_size, nullptr);
    return err == QuantizationError::SUCCESS;
}

QuantizationError AfricaQuant::dequantize_1_28bit(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    // Auto-detect block size from metadata if needed
    if (block_size == 0 && metadata) {
        block_size = metadata[0].block_size;
    }
    if (block_size == 0) {
        block_size = 128; // Default
    }
    
#ifdef KIPEPEO_NEON_ENABLED
    if (impl_->neon_enabled_) {
        return impl_->dequantize_1_28bit_neon(quantized, count, output, metadata, block_size);
    }
#endif
    return impl_->dequantize_1_28bit_scalar(quantized, count, output, metadata, block_size);
}

bool AfricaQuant::dequantize_1_28bit_legacy(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    QuantizationError err = dequantize_1_28bit(quantized, count, output, metadata, block_size);
    return err == QuantizationError::SUCCESS;
}

QuantizationError AfricaQuant::quantize_1_58bit(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size,
    const QuantizationConfig* config
) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    // Use config or defaults
    QuantizationConfig effective_config;
    if (config) {
        effective_config = *config;
    } else {
        effective_config = QuantizationConfig();
    }
    
    // Auto-detect block size if needed
    if (block_size == 0) {
        block_size = effective_config.block_size;
        if (block_size == 0) {
            block_size = get_optimal_block_size(count, effective_config.hardware.available_memory);
        }
    }
    
    const ProgressCallback* progress_cb = effective_config.progress_callback ? &effective_config.progress_callback : nullptr;
    
    return impl_->quantize_1_58bit_scalar(weights, count, output, metadata, block_size, progress_cb);
}

bool AfricaQuant::quantize_1_58bit_legacy(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size
) {
    QuantizationError err = quantize_1_58bit(weights, count, output, metadata, block_size, nullptr);
    return err == QuantizationError::SUCCESS;
}

QuantizationError AfricaQuant::dequantize_1_58bit(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    // Auto-detect block size from metadata if needed
    if (block_size == 0 && metadata) {
        block_size = metadata[0].block_size;
    }
    if (block_size == 0) {
        block_size = 128; // Default
    }
    
#ifdef KIPEPEO_NEON_ENABLED
    if (impl_->neon_enabled_) {
        return impl_->dequantize_1_58bit_neon(quantized, count, output, metadata, block_size);
    }
#endif
    return impl_->dequantize_1_58bit_scalar(quantized, count, output, metadata, block_size);
}

bool AfricaQuant::dequantize_1_58bit_legacy(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    QuantizationError err = dequantize_1_58bit(quantized, count, output, metadata, block_size);
    return err == QuantizationError::SUCCESS;
}

// Utility functions
size_t AfricaQuant::get_quantized_buffer_size(size_t count, float bits_per_weight) {
    return static_cast<size_t>(std::ceil(count * bits_per_weight / 8.0f)) + 16; // +16 for safety margin
}

size_t AfricaQuant::get_metadata_count(size_t count, uint32_t block_size) {
    return (count + block_size - 1) / block_size;
}

bool AfricaQuant::has_neon_support() const {
    return impl_->neon_enabled_;
}

void AfricaQuant::set_neon_enabled(bool enabled) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
#ifdef KIPEPEO_NEON_ENABLED
    impl_->neon_enabled_ = enabled;
#else
    impl_->neon_enabled_ = false;
#endif
}

const HardwareCapabilities& AfricaQuant::get_hardware_capabilities() const {
    return impl_->hardware_caps_;
}

void AfricaQuant::set_hardware_capabilities(const HardwareCapabilities& caps) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    impl_->hardware_caps_ = caps;
}

QuantizationError AfricaQuant::validate_inputs(
    const void* weights,
    size_t count,
    const void* output,
    const void* metadata,
    uint32_t block_size,
    size_t output_buffer_size
) {
    if (!weights) return QuantizationError::ERROR_NULL_POINTER;
    if (!output) return QuantizationError::ERROR_NULL_POINTER;
    if (!metadata) return QuantizationError::ERROR_NULL_POINTER;
    if (count == 0) return QuantizationError::ERROR_INVALID_COUNT;
    if (block_size == 0) return QuantizationError::ERROR_INVALID_BLOCK_SIZE;
    
    // Check if block_size is power of 2
    if ((block_size & (block_size - 1)) != 0) {
        return QuantizationError::ERROR_INVALID_BLOCK_SIZE;
    }
    
    // Check buffer size
    size_t required_size = AfricaQuant::get_quantized_buffer_size(count, 1.58f);
    if (output_buffer_size < required_size) {
        return QuantizationError::ERROR_INVALID_BUFFER_SIZE;
    }
    
    return QuantizationError::SUCCESS;
}

// Matrix quantization helpers
QuantizationError AfricaQuant::quantize_matrix_1_28bit(
    const float* weights,
    size_t M,
    size_t K,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size,
    const QuantizationConfig* config
) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    // Validate inputs
    if (!weights || !output || !metadata) {
        return QuantizationError::ERROR_NULL_POINTER;
    }
    if (M == 0 || K == 0) {
        return QuantizationError::ERROR_INVALID_COUNT;
    }
    
    // Use config or defaults
    QuantizationConfig effective_config;
    if (config) {
        effective_config = *config;
    } else {
        effective_config = QuantizationConfig();
    }
    
    // Auto-detect block size if needed
    if (block_size == 0) {
        block_size = effective_config.block_size;
        if (block_size == 0) {
            block_size = get_optimal_block_size(M * K, effective_config.hardware.available_memory);
        }
    }
    
    size_t num_blocks_per_row = (K + block_size - 1) / block_size;
    size_t quantized_bytes_per_row = (K * 2 + 7) / 8; // 2 bits per value
    
    // Get adaptive threshold
    float threshold = effective_config.threshold_1_28;
    if (threshold <= 0.0f && effective_config.use_adaptive_thresholds) {
        threshold = get_adaptive_threshold_1_28(weights, M * K, effective_config.hardware);
    } else if (threshold <= 0.0f) {
        threshold = effective_config.hardware.optimal_threshold_1_28;
    }
    
    const ProgressCallback* progress_cb = effective_config.progress_callback ? &effective_config.progress_callback : nullptr;
    
    // Quantize each row separately
    for (size_t row = 0; row < M; ++row) {
        const float* row_weights = weights + row * K;
        uint8_t* row_output = output + row * quantized_bytes_per_row;
        QuantizationMeta* row_metadata = metadata + row * num_blocks_per_row;
        
        QuantizationError err;
#ifdef KIPEPEO_NEON_ENABLED
        if (impl_->neon_enabled_) {
            err = impl_->quantize_1_28bit_neon(row_weights, K, row_output, row_metadata, block_size, threshold, progress_cb);
        } else {
            err = impl_->quantize_1_28bit_scalar(row_weights, K, row_output, row_metadata, block_size, threshold, progress_cb);
        }
#else
        err = impl_->quantize_1_28bit_scalar(row_weights, K, row_output, row_metadata, block_size, threshold, progress_cb);
#endif
        
        if (err != QuantizationError::SUCCESS) {
            return err;
        }
        
        // Update progress
        if (progress_cb && M > 10) {
            float progress = static_cast<float>(row + 1) / M;
            (*progress_cb)(progress);
        }
    }
    
    return QuantizationError::SUCCESS;
}

QuantizationError AfricaQuant::quantize_matrix_1_58bit(
    const float* weights,
    size_t M,
    size_t K,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size,
    const QuantizationConfig* config
) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    // Validate inputs
    if (!weights || !output || !metadata) {
        return QuantizationError::ERROR_NULL_POINTER;
    }
    if (M == 0 || K == 0) {
        return QuantizationError::ERROR_INVALID_COUNT;
    }
    
    // Use config or defaults
    QuantizationConfig effective_config;
    if (config) {
        effective_config = *config;
    } else {
        effective_config = QuantizationConfig();
    }
    
    // Auto-detect block size if needed
    if (block_size == 0) {
        block_size = effective_config.block_size;
        if (block_size == 0) {
            block_size = get_optimal_block_size(M * K, effective_config.hardware.available_memory);
        }
    }
    
    size_t num_blocks_per_row = (K + block_size - 1) / block_size;
    size_t quantized_bytes_per_row = (K * 2 + 7) / 8; // 2 bits per value
    
    const ProgressCallback* progress_cb = effective_config.progress_callback ? &effective_config.progress_callback : nullptr;
    
    // Quantize each row separately
    for (size_t row = 0; row < M; ++row) {
        const float* row_weights = weights + row * K;
        uint8_t* row_output = output + row * quantized_bytes_per_row;
        QuantizationMeta* row_metadata = metadata + row * num_blocks_per_row;
        
        QuantizationError err = impl_->quantize_1_58bit_scalar(row_weights, K, row_output, row_metadata, block_size, progress_cb);
        
        if (err != QuantizationError::SUCCESS) {
            return err;
        }
        
        // Update progress
        if (progress_cb && M > 10) {
            float progress = static_cast<float>(row + 1) / M;
            (*progress_cb)(progress);
        }
    }
    
    return QuantizationError::SUCCESS;
}

// Legacy API (delegates to 1.58-bit)
bool AfricaQuant::quantize(const float* weights, size_t count, uint8_t* output) {
    // Use default block size and allocate metadata
    uint32_t block_size = impl_->hardware_caps_.optimal_block_size;
    if (block_size == 0) block_size = 128;
    
    size_t num_blocks = get_metadata_count(count, block_size);
    QuantizationMeta* metadata = new QuantizationMeta[num_blocks];
    QuantizationError err = quantize_1_58bit(weights, count, output, metadata, block_size, nullptr);
    delete[] metadata;
    return err == QuantizationError::SUCCESS;
}

bool AfricaQuant::dequantize(const uint8_t* quantized, size_t count, float* output) {
    // Note: This won't work without metadata, kept for API compatibility
    // Real usage should use the explicit methods
    return false;
}

// Matrix-vector multiplication implementations using optimized kernels
QuantizationError AfricaQuant::matvec_mul_1_28bit(
    const uint8_t* quantized_A,
    const QuantizationMeta* metadata_A,
    const float* X,
    float* Y,
    size_t M,
    size_t K
) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    // Validate inputs
    if (!quantized_A || !metadata_A || !X || !Y) {
        return QuantizationError::ERROR_NULL_POINTER;
    }
    if (M == 0 || K == 0) {
        return QuantizationError::ERROR_INVALID_COUNT;
    }
    
    // Get block size from metadata
    uint32_t block_size = metadata_A[0].block_size;
    if (block_size == 0) {
        block_size = 128; // Default
    }
    
    // Calculate number of blocks per row
    size_t num_blocks_per_row = (K + block_size - 1) / block_size;
    
    // Extract scales from metadata
    // Metadata is organized as: metadata_A[row * num_blocks_per_row + block_idx]
    // Each metadata entry contains the scale for that specific row and block
    std::vector<float> scales(M * num_blocks_per_row);
    for (size_t row = 0; row < M; ++row) {
        for (size_t block = 0; block < num_blocks_per_row; ++block) {
            size_t metadata_idx = row * num_blocks_per_row + block;
            scales[row * num_blocks_per_row + block] = metadata_A[metadata_idx].scale;
            
            // Validate scale
            if (scales[row * num_blocks_per_row + block] <= 0.0f || 
                !std::isfinite(scales[row * num_blocks_per_row + block])) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }
        }
    }
    
    // Calculate quantized data offset per row
    // Each row's quantized data starts at: row * ((K * 2 + 7) / 8)
    size_t quantized_bytes_per_row = (K * 2 + 7) / 8; // 2 bits per value
    
    // Use optimized kernel from kernels module
    // The kernel expects quantized_A to be organized row-major
    kernels::neon::gemv_ternary_1_28bit(
        M, K,
        1.0f,  // alpha
        quantized_A,  // Row-major quantized matrix
        scales.data(),  // Scales organized as scales[row * num_blocks_per_row + block]
        X,
        0.0f,  // beta (overwrite Y)
        Y,
        block_size
    );
    
    return QuantizationError::SUCCESS;
}

QuantizationError AfricaQuant::matvec_mul_1_58bit(
    const uint8_t* quantized_A,
    const QuantizationMeta* metadata_A,
    const float* X,
    float* Y,
    size_t M,
    size_t K
) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    
    // Validate inputs
    if (!quantized_A || !metadata_A || !X || !Y) {
        return QuantizationError::ERROR_NULL_POINTER;
    }
    if (M == 0 || K == 0) {
        return QuantizationError::ERROR_INVALID_COUNT;
    }
    
    // Get block size from metadata
    uint32_t block_size = metadata_A[0].block_size;
    if (block_size == 0) {
        block_size = 128; // Default
    }
    
    // Calculate number of blocks per row
    size_t num_blocks_per_row = (K + block_size - 1) / block_size;
    
    // Extract scales from metadata
    // Metadata is organized as: metadata_A[row * num_blocks_per_row + block_idx]
    // Each metadata entry contains the scale for that specific row and block
    std::vector<float> scales(M * num_blocks_per_row);
    for (size_t row = 0; row < M; ++row) {
        for (size_t block = 0; block < num_blocks_per_row; ++block) {
            size_t metadata_idx = row * num_blocks_per_row + block;
            scales[row * num_blocks_per_row + block] = metadata_A[metadata_idx].scale;
            
            // Validate scale
            if (scales[row * num_blocks_per_row + block] <= 0.0f || 
                !std::isfinite(scales[row * num_blocks_per_row + block])) {
                return QuantizationError::ERROR_INVALID_SCALE;
            }
        }
    }
    
    // Calculate quantized data offset per row
    // Each row's quantized data starts at: row * ((K * 2 + 7) / 8)
    size_t quantized_bytes_per_row = (K * 2 + 7) / 8; // 2 bits per value
    
    // Use optimized kernel from kernels module
    // The kernel expects quantized_A to be organized row-major
    kernels::neon::gemv_quaternary_1_58bit(
        M, K,
        1.0f,  // alpha
        quantized_A,  // Row-major quantized matrix
        scales.data(),  // Scales organized as scales[row * num_blocks_per_row + block]
        X,
        0.0f,  // beta (overwrite Y)
        Y,
        block_size
    );
    
    return QuantizationError::SUCCESS;
}

// ========== C API Implementation ==========

extern "C" {

bool kipepeo_quantize_1_28bit(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size
) {
    AfricaQuant quant;
    QuantizationError err = quant.quantize_1_28bit(weights, count, output, metadata, block_size, nullptr);
    return err == QuantizationError::SUCCESS;
}

bool kipepeo_dequantize_1_28bit(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    AfricaQuant quant;
    QuantizationError err = quant.dequantize_1_28bit(quantized, count, output, metadata, block_size);
    return err == QuantizationError::SUCCESS;
}

bool kipepeo_quantize_1_58bit(
    const float* weights,
    size_t count,
    uint8_t* output,
    QuantizationMeta* metadata,
    uint32_t block_size
) {
    AfricaQuant quant;
    QuantizationError err = quant.quantize_1_58bit(weights, count, output, metadata, block_size, nullptr);
    return err == QuantizationError::SUCCESS;
}

bool kipepeo_dequantize_1_58bit(
    const uint8_t* quantized,
    size_t count,
    float* output,
    const QuantizationMeta* metadata,
    uint32_t block_size
) {
    AfricaQuant quant;
    QuantizationError err = quant.dequantize_1_58bit(quantized, count, output, metadata, block_size);
    return err == QuantizationError::SUCCESS;
}

} // extern "C"

} // namespace quantization
} // namespace kipepeo
