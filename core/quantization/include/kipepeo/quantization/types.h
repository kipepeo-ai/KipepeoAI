#pragma once

namespace kipepeo {
namespace quantization {

// Quantization types
enum class QuantType {
    Q1_58_BIT,  // 1.58-bit quantization (Q1_K_M)
    Q1_BIT,     // 1-bit quantization (AfricaQuant)
    Q4_BIT,
    Q8_BIT
};

} // namespace quantization
} // namespace kipepeo

