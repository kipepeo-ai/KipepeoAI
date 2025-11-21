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

# Add subdirectories
add_subdirectory(core)

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

