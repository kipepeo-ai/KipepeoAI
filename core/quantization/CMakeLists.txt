# Quantization Library (AfricaQuant)

set(QUANTIZATION_SOURCES
    src/africa_quant.cpp
    src/quantization_utils.cpp
)

set(QUANTIZATION_HEADERS
    include/kipepeo/quantization/africa_quant.h
    include/kipepeo/quantization/quantization_utils.h
    include/kipepeo/quantization/types.h
)

# Create library
add_library(kipepeo_quantization STATIC
    ${QUANTIZATION_SOURCES}
    ${QUANTIZATION_HEADERS}
)

target_include_directories(kipepeo_quantization PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(kipepeo_quantization PUBLIC
    kipepeo_kernels
)

# Compile definitions
target_compile_definitions(kipepeo_quantization PRIVATE
    KIPEPEO_QUANTIZATION_BUILD
)

