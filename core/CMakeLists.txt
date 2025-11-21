cmake_minimum_required(VERSION 3.18)

project(KipepeoCore)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(IOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")
    add_compile_definitions(KIPEPEO_IOS)
endif()

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(video)
add_subdirectory(llm)
add_subdirectory(quantization)
add_subdirectory(kernels)

if(IOS)
    add_subdirectory(ios)
endif()

# New Voice Call Module
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/voice_call")
    add_subdirectory(src/voice_call)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/vision")
    add_subdirectory(src/vision)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/health")
    add_subdirectory(src/health)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/education")
    add_subdirectory(src/education)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/finance")
    add_subdirectory(src/finance)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/mesh")
    add_subdirectory(src/mesh)
endif()
