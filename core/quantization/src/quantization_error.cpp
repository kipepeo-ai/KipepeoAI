#include "kipepeo/quantization/quantization_error.h"

namespace kipepeo {
namespace quantization {

const char* get_error_message(QuantizationError error) {
    switch (error) {
        case QuantizationError::SUCCESS:
            return "Success";
        case QuantizationError::ERROR_NULL_POINTER:
            return "Null pointer provided";
        case QuantizationError::ERROR_INVALID_COUNT:
            return "Invalid count (must be > 0)";
        case QuantizationError::ERROR_INVALID_BLOCK_SIZE:
            return "Invalid block size (must be > 0 and power of 2)";
        case QuantizationError::ERROR_INVALID_BUFFER_SIZE:
            return "Invalid buffer size (insufficient space)";
        case QuantizationError::ERROR_INVALID_METADATA:
            return "Invalid metadata pointer or size";
        case QuantizationError::ERROR_BUFFER_OVERFLOW:
            return "Buffer overflow detected";
        case QuantizationError::ERROR_INSUFFICIENT_MEMORY:
            return "Insufficient memory";
        case QuantizationError::ERROR_MEMORY_ALIGNMENT:
            return "Memory alignment error (NEON requires 16-byte alignment)";
        case QuantizationError::ERROR_QUANTIZATION_FAILED:
            return "Quantization operation failed";
        case QuantizationError::ERROR_DEQUANTIZATION_FAILED:
            return "Dequantization operation failed";
        case QuantizationError::ERROR_INVALID_SCALE:
            return "Invalid scale value (must be > 0)";
        case QuantizationError::ERROR_INVALID_QUANTIZED_DATA:
            return "Invalid quantized data format";
        case QuantizationError::ERROR_NEON_NOT_AVAILABLE:
            return "NEON not available on this hardware";
        case QuantizationError::ERROR_HARDWARE_DETECTION_FAILED:
            return "Hardware detection failed";
        case QuantizationError::ERROR_INVALID_CONFIG:
            return "Invalid quantization configuration";
        case QuantizationError::ERROR_UNSUPPORTED_BLOCK_SIZE:
            return "Unsupported block size";
        case QuantizationError::ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

} // namespace quantization
} // namespace kipepeo

