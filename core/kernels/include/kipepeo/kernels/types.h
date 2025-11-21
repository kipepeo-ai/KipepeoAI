#pragma once

#include <cstddef>
#include <cstdint>

namespace kipepeo {
namespace kernels {

// Matrix dimensions
struct MatrixDim {
    size_t rows;
    size_t cols;
};

// Performance metrics for kernels
struct KernelMetrics {
    float gflops = 0.0f;
    size_t memory_bandwidth_mbps = 0;
    float cache_hit_rate = 0.0f;
};

} // namespace kernels
} // namespace kipepeo

