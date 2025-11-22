# Optimized Kernels Library (NEON, MediaTek, Metal)

set(KERNELS_SOURCES
    src/neon/matrix_multiply.cpp
    src/neon/vector_ops.cpp
    src/neon/quantized_gemm.cpp
    src/chip_detection.cpp
    src/kernel_dispatch.cpp
    # MediaTek Helio series
    src/mediatek/helio_optimizations.cpp
    src/mediatek/helio_g85.cpp
    src/mediatek/helio_g99.cpp
    src/mediatek/helio_g100.cpp
    # Qualcomm Snapdragon series
    src/qualcomm/snapdragon_7s_gen2.cpp
    src/qualcomm/snapdragon_common.cpp
    # Unisoc series
    src/unisoc/t606.cpp
    # Apple Silicon
    src/apple/neon_apple.cpp
)

set(KERNELS_HEADERS
    include/kipepeo/kernels/neon/matrix_multiply.h
    include/kipepeo/kernels/neon/vector_ops.h
    include/kipepeo/kernels/neon/quantized_gemm.h
    include/kipepeo/kernels/chip_detection.h
    include/kipepeo/kernels/kernel_dispatch.h
    include/kipepeo/kernels/types.h
    # MediaTek Helio series
    include/kipepeo/kernels/mediatek/helio_optimizations.h
    include/kipepeo/kernels/mediatek/helio_g85.h
    include/kipepeo/kernels/mediatek/helio_g99.h
    include/kipepeo/kernels/mediatek/helio_g100.h
    # Qualcomm Snapdragon series
    include/kipepeo/kernels/qualcomm/snapdragon_7s_gen2.h
    include/kipepeo/kernels/qualcomm/snapdragon_common.h
    # Unisoc series
    include/kipepeo/kernels/unisoc/t606.h
    # Apple Silicon
    include/kipepeo/kernels/apple/neon_apple.h
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
