# Optimized Kernels Library (NEON, MediaTek, Metal)

set(KERNELS_SOURCES
    src/neon/matrix_multiply.cpp
    src/neon/vector_ops.cpp
    src/neon/quantized_gemm.cpp
    src/mediatek/helio_optimizations.cpp
)

set(KERNELS_HEADERS
    include/kipepeo/kernels/neon/matrix_multiply.h
    include/kipepeo/kernels/neon/vector_ops.h
    include/kipepeo/kernels/neon/quantized_gemm.h
    include/kipepeo/kernels/mediatek/helio_optimizations.h
    include/kipepeo/kernels/types.h
)

set(KERNEL_LIBS "")

# Metal backend for Apple platforms
if(APPLE)
    set(METAL_SOURCES
        metal/ggml-metal.m
        metal/ggml-metal.metal
    )
    
    # Find Metal libraries
    find_library(METAL_LIBRARY Metal)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(METALKIT_LIBRARY MetalKit)
    
    list(APPEND KERNEL_SOURCES ${METAL_SOURCES})
    list(APPEND KERNEL_LIBS ${METAL_LIBRARY} ${FOUNDATION_LIBRARY} ${METALKIT_LIBRARY})
    
    # Set compile flags for Obj-C++
    set_source_files_properties(metal/ggml-metal.m PROPERTIES LANGUAGE CXX)
endif()

# Create library
add_library(kipepeo_kernels STATIC
    ${KERNELS_SOURCES}
    ${KERNELS_HEADERS}
    ${METAL_SOURCES}
)

target_include_directories(kipepeo_kernels PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
if(APPLE)
    target_link_libraries(kipepeo_kernels PUBLIC ${KERNEL_LIBS})
    target_compile_options(kipepeo_kernels PRIVATE "-fobjc-arc")
endif()

# NEON intrinsics
if(KIPEPEO_NEON_ENABLED)
    target_compile_definitions(kipepeo_kernels PRIVATE KIPEPEO_NEON_ENABLED)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armeabi-v7a")
        target_compile_options(kipepeo_kernels PRIVATE -mfpu=neon)
    endif()
endif()

# Compile definitions
target_compile_definitions(kipepeo_kernels PRIVATE
    KIPEPEO_KERNELS_BUILD
)
