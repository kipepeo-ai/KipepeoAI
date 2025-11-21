# Android NDK Library - libkipepeo

set(ANDROID_SOURCES
    src/main/cpp/jni_bridge.cpp
    src/main/cpp/native_interface.cpp
)

set(ANDROID_HEADERS
    src/main/cpp/native_interface.h
)

# Create shared library for Android
add_library(kipepeo_android SHARED
    ${ANDROID_SOURCES}
    ${ANDROID_HEADERS}
)

target_include_directories(kipepeo_android PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp
    ${CMAKE_SOURCE_DIR}/core/llm/include
    ${CMAKE_SOURCE_DIR}/core/video/include
    ${CMAKE_SOURCE_DIR}/core/kernels/include
    ${CMAKE_SOURCE_DIR}/core/quantization/include
    ${CMAKE_SOURCE_DIR}/core/src/voice_call
    ${CMAKE_SOURCE_DIR}/core/src/vision
)

# Link core libraries
target_link_libraries(kipepeo_android PRIVATE
    kipepeo_llm
    kipepeo_video
    kipepeo_kernels
    kipepeo_quantization
    kipepeo_voice
    kipepeo_vision
)

# Android-specific compile definitions
target_compile_definitions(kipepeo_android PRIVATE
    KIPEPEO_ANDROID_BUILD
    JNI_IMPLEMENTATION
)

# Set library output name
set_target_properties(kipepeo_android PROPERTIES
    OUTPUT_NAME "kipepeo"
    PREFIX "lib"
)

