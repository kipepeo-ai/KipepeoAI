# Video Compression/Encoding Library

set(VIDEO_SOURCES
    src/av1_encoder.cpp
    src/av1_decoder.cpp
    src/video_compressor.cpp
    src/rate_control.cpp
)

set(VIDEO_HEADERS
    include/kipepeo/video/av1_encoder.h
    include/kipepeo/video/av1_decoder.h
    include/kipepeo/video/video_compressor.h
    include/kipepeo/video/rate_control.h
    include/kipepeo/video/types.h
)

# Create library
add_library(kipepeo_video STATIC
    ${VIDEO_SOURCES}
    ${VIDEO_HEADERS}
)

target_include_directories(kipepeo_video PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(kipepeo_video PUBLIC
    kipepeo_kernels
)

# Link third-party dependencies when available
if(TARGET rav1e)
    target_link_libraries(kipepeo_video PRIVATE rav1e)
endif()

if(TARGET dav1d)
    target_link_libraries(kipepeo_video PRIVATE dav1d)
endif()

# Compile definitions
target_compile_definitions(kipepeo_video PRIVATE
    KIPEPEO_VIDEO_BUILD
)

