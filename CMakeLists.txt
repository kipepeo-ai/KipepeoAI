cmake_minimum_required(VERSION 3.22)

project(Kipepeo
    VERSION 1.0.0
    DESCRIPTION "Ultra-efficient Offline AI + Video Compression Stack for Low-end Android Phones"
    LANGUAGES CXX C
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build options
option(KIPEPEO_BUILD_LLM "Build LLM inference engine" ON)
option(KIPEPEO_BUILD_VIDEO "Build video compression/encoding" ON)
option(KIPEPEO_BUILD_KERNELS "Build optimized kernels (NEON, MediaTek)" ON)
option(KIPEPEO_BUILD_QUANTIZATION "Build quantization tools" ON)
option(KIPEPEO_BUILD_ANDROID "Build Android NDK library" ON)
option(KIPEPEO_BUILD_TESTS "Build tests" OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include cmake utilities
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Android-specific configuration
if(ANDROID)
    include(cmake/Android.cmake)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Enable warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    # Optimization flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
        # Size optimization for mobile
        if(ANDROID)
            add_compile_options(-Os)
        endif()
    endif()
    
    # ARM NEON support
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM|aarch64|AARCH64")
        add_compile_definitions(KIPEPEO_NEON_ENABLED)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armv7-a|armeabi-v7a")
            add_compile_options(-mfpu=neon)
        endif()
    endif()
    
    # MediaTek-specific optimizations
    if(ANDROID)
        add_compile_definitions(KIPEPEO_MEDIATEK_OPTIMIZED)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/core/llm/include
    ${CMAKE_SOURCE_DIR}/core/video/include
    ${CMAKE_SOURCE_DIR}/core/kernels/include
    ${CMAKE_SOURCE_DIR}/core/quantization/include
)

#Add third-party dependencies
# llama.cpp integration
option(LLAMA_BUILD_TESTS "llama: build tests" OFF)
option(LLAMA_BUILD_EXAMPLES "llama: build examples" OFF)
option(LLAMA_BUILD_SERVER "llama: build server" OFF)
option(LLAMA_BUILD_TOOLS "llama: build tools" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

if(ANDROID)
    # For Android, we want static library
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    
    # ARM NEON optimizations (enabled by default on ARM)
    # llama.cpp will automatically detect and use NEON on ARM processors
    message(STATUS "Android build detected - NEON optimizations will be auto-enabled")
    
    # Optimize for size and speed on mobile
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -ffast-math")
endif()

# Performance optimizations for llama.cpp
# Enable CPU backend (includes NEON for ARM)
set(GGML_CPU ON CACHE BOOL "Enable CPU backend with NEON" FORCE)

# Disable backends we don't need on mobile
set(GGML_CUDA OFF CACHE BOOL "Disable CUDA" FORCE)
set(GGML_METAL OFF CACHE BOOL "Disable Metal" FORCE)
set(GGML_VULKAN OFF CACHE BOOL "Disable Vulkan" FORCE)
set(GGML_SYCL OFF CACHE BOOL "Disable SYCL" FORCE)

add_subdirectory(third_party/llama.cpp EXCLUDE_FROM_ALL)

# Add subdirectories
add_subdirectory(core)

# Create unified core library for Android NDK
if(NOT ANDROID)
    # Only create the unified target for non-Android builds
    # Android builds will link libraries directly
    add_library(kipepeo_core INTERFACE)
    target_link_libraries(kipepeo_core INTERFACE
        kipepeo_kernels
        kipepeo_quantization
        kipepeo_llm
        kipepeo_video
    )
    target_include_directories(kipepeo_core INTERFACE
        ${CMAKE_SOURCE_DIR}/core/llm/include
        ${CMAKE_SOURCE_DIR}/core/video/include
        ${CMAKE_SOURCE_DIR}/core/kernels/include
        ${CMAKE_SOURCE_DIR}/core/quantization/include
    )
endif()

if(KIPEPEO_BUILD_ANDROID)
    add_subdirectory(android)
endif()

if(KIPEPEO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== Kipepeo Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build LLM: ${KIPEPEO_BUILD_LLM}")
message(STATUS "Build Video: ${KIPEPEO_BUILD_VIDEO}")
message(STATUS "Build Kernels: ${KIPEPEO_BUILD_KERNELS}")
message(STATUS "Build Quantization: ${KIPEPEO_BUILD_QUANTIZATION}")
message(STATUS "Build Android: ${KIPEPEO_BUILD_ANDROID}")
message(STATUS "Build Tests: ${KIPEPEO_BUILD_TESTS}")
if(ANDROID)
    message(STATUS "Android API: ${ANDROID_PLATFORM_LEVEL}")
    message(STATUS "Android ABI: ${ANDROID_ABI}")
endif()
message(STATUS "================================")
message(STATUS "")

