#pragma once

#include "kipepeo/quantization/types.h"

namespace kipepeo {
namespace quantization {

/**
 * AfricaQuant - Custom 1-bit quantization optimized for African languages
 * (Swahili, English, Sheng) and low-end Android devices
 */
class AfricaQuant {
public:
    AfricaQuant();
    ~AfricaQuant();

    // Quantize model weights
    bool quantize(const float* weights, size_t count, uint8_t* output);

    // Dequantize for inference
    bool dequantize(const uint8_t* quantized, size_t count, float* output);

private:
    class Impl;
    Impl* impl_;
};

} // namespace quantization
} // namespace kipepeo

