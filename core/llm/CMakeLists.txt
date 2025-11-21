# LLM Inference Engine Library

set(LLM_SOURCES
    src/llm_engine.cpp
    src/model_loader.cpp
    src/inference.cpp
)

set(LLM_HEADERS
    include/kipepeo/llm/llm_engine.h
    include/kipepeo/llm/model_loader.h
    include/kipepeo/llm/inference.h
    include/kipepeo/llm/types.h
)

# Create library
add_library(kipepeo_llm STATIC
    ${LLM_SOURCES}
    ${LLM_HEADERS}
)

target_include_directories(kipepeo_llm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(kipepeo_llm PUBLIC
    kipepeo_kernels
)

# Link third-party dependencies when available
if(TARGET llama_cpp)
    target_link_libraries(kipepeo_llm PRIVATE llama_cpp)
endif()

# Compile definitions
target_compile_definitions(kipepeo_llm PRIVATE
    KIPEPEO_LLM_BUILD
)

