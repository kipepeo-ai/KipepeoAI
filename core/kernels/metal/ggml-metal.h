#pragma once

#include "ggml.h"

#ifdef __cplusplus
extern "C" {
#endif

void ggml_metal_init(void);
void ggml_metal_free(void);

void * ggml_metal_host_malloc(size_t n);
void   ggml_metal_host_free(void * ptr);

bool ggml_metal_add_buffer(void * addr, size_t size);
void ggml_metal_graph_compute(struct ggml_cgraph * gf);

#ifdef __cplusplus
}
#endif
