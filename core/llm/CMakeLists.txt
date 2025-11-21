# LLM Inference Engine Library

set(LLM_SOURCES
    src/llm_engine.cpp
    src/model_loader.cpp
    src/inference.cpp
    src/model_switcher.cpp
    src/llama_integration.cpp
)

set(LLM_HEADERS
    include/kipepeo/llm/llm_engine.h
    include/kipepeo/llm/model_loader.h
    include/kipepeo/llm/inference.h
    include/kipepeo/llm/types.h
    include/kipepeo/llm/model_switcher.h
    include/kipepeo/llm/llama_integration.h
)

# Create library
add_library(kipepeo_llm STATIC
    ${LLM_SOURCES}
    ${LLM_HEADERS}
)

target_include_directories(kipepeo_llm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/llama.cpp/include
    ${CMAKE_SOURCE_DIR}/third_party/llama.cpp/ggml/include
)

target_link_libraries(kipepeo_llm PUBLIC
    kipepeo_kernels
    llama
)

# Link third-party dependencies when available
# llama library is always linked above

# Compile definitions
target_compile_definitions(kipepeo_llm PRIVATE
    KIPEPEO_LLM_BUILD
)

