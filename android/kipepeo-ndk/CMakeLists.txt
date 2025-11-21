cmake_minimum_required(VERSION 3.22.1)

project(kipepeo_ndk)

# Add xhook library
add_subdirectory(${CMAKE_SOURCE_DIR}/../../third_party/xhook ${CMAKE_BINARY_DIR}/xhook)

# Source files
set(KIPEPEO_NDK_SOURCES
    src/main/cpp/jni_bridge.cpp
    src/main/cpp/native_interface.cpp
    src/main/cpp/hook_manager.cpp
    src/main/cpp/mediacodec_hooks.cpp
    src/main/cpp/data_tracker.cpp
)

# Header files
set(KIPEPEO_NDK_HEADERS
    src/main/cpp/native_interface.h
    src/main/cpp/hook_manager.h
    src/main/cpp/mediacodec_hooks.h
    src/main/cpp/data_tracker.h
)

# Create shared library
add_library(kipepeo SHARED
    ${KIPEPEO_NDK_SOURCES}
    ${KIPEPEO_NDK_HEADERS}
)

# Include directories
target_include_directories(kipepeo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp
    ${CMAKE_SOURCE_DIR}/../../core/llm/include
    ${CMAKE_SOURCE_DIR}/../../core/video/include
    ${CMAKE_SOURCE_DIR}/../../core/kernels/include
    ${CMAKE_SOURCE_DIR}/../../core/quantization/include
    ${CMAKE_SOURCE_DIR}/../../third_party/xhook/libxhook/jni
)

# Link libraries
target_link_libraries(kipepeo
    PRIVATE
        xhook
        log
        android
)

# Conditionally link core libraries if they exist
# In full monorepo build, these will be available
# For standalone NDK build, we'll handle externally
if(TARGET kipepeo_llm)
    target_link_libraries(kipepeo PRIVATE kipepeo_llm)
    target_compile_definitions(kipepeo PRIVATE KIPEPEO_HAS_LLM)
endif()

if(TARGET kipepeo_video)
    target_link_libraries(kipepeo PRIVATE kipepeo_video)
    target_compile_definitions(kipepeo PRIVATE KIPEPEO_HAS_VIDEO)
endif()

if(TARGET kipepeo_kernels)
    target_link_libraries(kipepeo PRIVATE kipepeo_kernels)
    target_compile_definitions(kipepeo PRIVATE KIPEPEO_HAS_KERNELS)
endif()

if(TARGET kipepeo_quantization)
    target_link_libraries(kipepeo PRIVATE kipepeo_quantization)
    target_compile_definitions(kipepeo PRIVATE KIPEPEO_HAS_QUANTIZATION)
endif()

# Android-specific compile definitions
target_compile_definitions(kipepeo PRIVATE
    KIPEPEO_ANDROID_BUILD
    JNI_IMPLEMENTATION
)

# Compiler options
target_compile_options(kipepeo PRIVATE
    -Wall
    -Wextra
    -fvisibility=hidden
    -ffunction-sections
    -fdata-sections
)

# Linker options for smaller binary
target_link_options(kipepeo PRIVATE
    -Wl,--gc-sections
    -Wl,--exclude-libs,ALL
)

# Set output name
set_target_properties(kipepeo PROPERTIES
    OUTPUT_NAME "kipepeo"
)
