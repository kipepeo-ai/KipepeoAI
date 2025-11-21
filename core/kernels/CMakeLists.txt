# Optimized Kernels Library (NEON, MediaTek)

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

# Create library
add_library(kipepeo_kernels STATIC
    ${KERNELS_SOURCES}
    ${KERNELS_HEADERS}
)

target_include_directories(kipepeo_kernels PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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

