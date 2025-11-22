#pragma once

#include <cstdint>

namespace kipepeo {
namespace quantization {

/**
 * Error codes for quantization operations
 */
enum class QuantizationError : int32_t {
    SUCCESS = 0,
    
    // Input validation errors
    ERROR_NULL_POINTER = -1,
    ERROR_INVALID_COUNT = -2,
    ERROR_INVALID_BLOCK_SIZE = -3,
    ERROR_INVALID_BUFFER_SIZE = -4,
    ERROR_INVALID_METADATA = -5,
    
    // Memory errors
    ERROR_BUFFER_OVERFLOW = -10,
    ERROR_INSUFFICIENT_MEMORY = -11,
    ERROR_MEMORY_ALIGNMENT = -12,
    
    // Quantization errors
    ERROR_QUANTIZATION_FAILED = -20,
    ERROR_DEQUANTIZATION_FAILED = -21,
    ERROR_INVALID_SCALE = -22,
    ERROR_INVALID_QUANTIZED_DATA = -23,
    
    // Hardware errors
    ERROR_NEON_NOT_AVAILABLE = -30,
    ERROR_HARDWARE_DETECTION_FAILED = -31,
    
    // Configuration errors
    ERROR_INVALID_CONFIG = -40,
    ERROR_UNSUPPORTED_BLOCK_SIZE = -41,
    
    // Unknown error
    ERROR_UNKNOWN = -100
};

/**
 * Get error message string for error code
 */
const char* get_error_message(QuantizationError error);

/**
 * Check if error code indicates success
 */
inline bool is_success(QuantizationError error) {
    return error == QuantizationError::SUCCESS;
}

/**
 * Check if error code indicates failure
 */
inline bool is_error(QuantizationError error) {
    return error != QuantizationError::SUCCESS;
}

} // namespace quantization
} // namespace kipepeo

