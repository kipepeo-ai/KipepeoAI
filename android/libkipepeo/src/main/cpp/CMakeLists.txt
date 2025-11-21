# Android NDK Build Entry Point
# This CMakeLists.txt is invoked by Android Gradle Plugin
# It includes the root project and builds libkipepeo.so

cmake_minimum_required(VERSION 3.22)

# Navigate to root project directory (5 levels up from src/main/cpp)
set(KIPEPEO_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

# Include root CMakeLists.txt which builds all core libraries
add_subdirectory(${KIPEPEO_ROOT_DIR}/core ${CMAKE_CURRENT_BINARY_DIR}/core)

# Now build the Android shared library
set(ANDROID_SOURCES
    jni_bridge.cpp
    native_interface.cpp
)

set(ANDROID_HEADERS
    native_interface.h
)

# Create shared library for Android
add_library(kipepeo SHARED
    ${ANDROID_SOURCES}
    ${ANDROID_HEADERS}
)

target_include_directories(kipepeo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KIPEPEO_ROOT_DIR}/core/llm/include
    ${KIPEPEO_ROOT_DIR}/core/video/include
    ${KIPEPEO_ROOT_DIR}/core/kernels/include
    ${KIPEPEO_ROOT_DIR}/core/quantization/include
)

# Link all core static libraries into the shared library
target_link_libraries(kipepeo PRIVATE
    kipepeo_kernels
    kipepeo_quantization
    kipepeo_llm
    kipepeo_video
    android
    log
)

# Android-specific compile definitions
target_compile_definitions(kipepeo PRIVATE
    KIPEPEO_ANDROID_BUILD
    JNI_IMPLEMENTATION
)

