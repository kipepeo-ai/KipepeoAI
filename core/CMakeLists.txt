cmake_minimum_required(VERSION 3.18)

project(kipepeo_core)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(video)
add_subdirectory(llm)
add_subdirectory(quantization)
add_subdirectory(kernels)

# New Voice Call Module
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/voice_call")
    add_subdirectory(src/voice_call)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/vision")
    add_subdirectory(src/vision)
endif()
