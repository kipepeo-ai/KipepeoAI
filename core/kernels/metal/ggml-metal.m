#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "ggml-metal.h"
#include "ggml.h"

#include <map>
#include <vector>
#include <mutex>

// Forward declaration of internal structures
struct ggml_metal_context {
    id<MTLDevice>       device;
    id<MTLCommandQueue> queue;
    id<MTLLibrary>      library;
    
    // Pipeline state objects for our kernels
    id<MTLComputePipelineState> pipeline_add;
    id<MTLComputePipelineState> pipeline_mul;
    id<MTLComputePipelineState> pipeline_mul_mat_f32_f32;
    id<MTLComputePipelineState> pipeline_softmax;
    
    // Buffers map: host pointer -> metal buffer
    std::map<void *, id<MTLBuffer>> buffers;
    std::mutex mutex;
};

// Global context for simplicity in this port
static ggml_metal_context * g_ctx = nullptr;

extern "C" {

void ggml_metal_init(void) {
    if (g_ctx) return;
    
    g_ctx = new ggml_metal_context();
    
    // 1. Pick the default device
    g_ctx->device = MTLCreateSystemDefaultDevice();
    if (!g_ctx->device) {
        fprintf(stderr, "%s: error: no Metal device found\n", __func__);
        return;
    }
    
    fprintf(stderr, "%s: using device: %s\n", __func__, [[g_ctx->device name] UTF8String]);
    
    // 2. Create command queue
    g_ctx->queue = [g_ctx->device newCommandQueue];
    
    // 3. Load library
    // In a real app, we might load from a .metallib file or compile source at runtime.
    // For this port, we assume the default library contains our kernels.
    g_ctx->library = [g_ctx->device newDefaultLibrary];
    if (!g_ctx->library) {
        fprintf(stderr, "%s: error: could not load default Metal library\n", __func__);
        // Try loading from bundle if default fails (common in frameworks)
        NSBundle * bundle = [NSBundle bundleForClass:[NSObject class]]; // Placeholder
        NSError * error = nil;
        g_ctx->library = [g_ctx->device newDefaultLibraryWithBundle:bundle error:&error];
        if (error) {
             fprintf(stderr, "%s: error loading library: %s\n", __func__, [[error description] UTF8String]);
        }
    }
    
    // 4. Create pipeline states
    NSError * error = nil;
    
    auto load_pipeline = [&](const char * name) -> id<MTLComputePipelineState> {
        id<MTLFunction> func = [g_ctx->library newFunctionWithName:[NSString stringWithUTF8String:name]];
        if (!func) {
            fprintf(stderr, "%s: error: function '%s' not found\n", __func__, name);
            return nil;
        }
        
        id<MTLComputePipelineState> pso = [g_ctx->device newComputePipelineStateWithFunction:func error:&error];
        if (error) {
            fprintf(stderr, "%s: error: could not create pipeline for '%s': %s\n", __func__, name, [[error description] UTF8String]);
            return nil;
        }
        return pso;
    };
    
    g_ctx->pipeline_add = load_pipeline("kernel_add");
    g_ctx->pipeline_mul = load_pipeline("kernel_mul");
    g_ctx->pipeline_mul_mat_f32_f32 = load_pipeline("kernel_mul_mat_f32_f32");
    g_ctx->pipeline_softmax = load_pipeline("kernel_softmax");
}

void ggml_metal_free(void) {
    if (!g_ctx) return;
    delete g_ctx;
    g_ctx = nullptr;
}

void * ggml_metal_host_malloc(size_t n) {
    // Allocate page-aligned memory for better Metal compatibility if needed
    void * ptr = nullptr;
    posix_memalign(&ptr, 4096, n);
    return ptr;
}

void ggml_metal_host_free(void * ptr) {
    free(ptr);
}

// Register a host buffer with Metal
bool ggml_metal_add_buffer(void * addr, size_t size) {
    if (!g_ctx) return false;
    
    std::lock_guard<std::mutex> lock(g_ctx->mutex);
    
    // Check if already exists
    if (g_ctx->buffers.find(addr) != g_ctx->buffers.end()) {
        return true;
    }
    
    // Create a Metal buffer that shares memory with the CPU (Shared Storage Mode)
    // This is efficient for unified memory architectures (Apple Silicon)
    id<MTLBuffer> buffer = [g_ctx->device newBufferWithBytesNoCopy:addr
                                                            length:size
                                                           options:MTLResourceStorageModeShared
                                                       deallocator:nil];
    
    if (!buffer) {
        fprintf(stderr, "%s: error: failed to create Metal buffer\n", __func__);
        return false;
    }
    
    g_ctx->buffers[addr] = buffer;
    return true;
}

// The main compute graph execution function
void ggml_metal_graph_compute(struct ggml_cgraph * gf) {
    if (!g_ctx) return;
    
    @autoreleasepool {
        id<MTLCommandBuffer> command_buffer = [g_ctx->queue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];
        
        for (int i = 0; i < gf->n_nodes; i++) {
            struct ggml_tensor * node = gf->nodes[i];
            
            // Find Metal buffers for src and dst
            // This is a simplified lookup. In reality, we need to handle offsets.
            // For this stub, we assume tensors are fully backed by registered buffers.
            
            // Example dispatch for ADD
            if (node->op == GGML_OP_ADD) {
                [encoder setComputePipelineState:g_ctx->pipeline_add];
                
                // Bind buffers (simplified)
                // [encoder setBuffer:src0_buf offset:src0_off atIndex:0];
                // [encoder setBuffer:src1_buf offset:src1_off atIndex:1];
                // [encoder setBuffer:dst_buf  offset:dst_off  atIndex:2];
                
                // Dispatch threads
                // MTLSize threadGroupSize = MTLSizeMake(256, 1, 1);
                // MTLSize gridSize = MTLSizeMake(n_elements, 1, 1);
                // [encoder dispatchThreads:gridSize threadsPerThreadgroup:threadGroupSize];
            }
            
            // ... Handle other ops ...
        }
        
        [encoder endEncoding];
        [command_buffer commit];
        [command_buffer waitUntilCompleted];
    }
}

} // extern "C"
