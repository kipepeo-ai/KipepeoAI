# Video Compression/Encoding Library

set(VIDEO_SOURCES
    src/av1_encoder.cpp
    src/av1_decoder.cpp
    src/video_compressor.cpp
    src/rate_control.cpp
    src/kip_mode_lowband.cpp
    src/rav1e_integration.cpp
)

set(VIDEO_HEADERS
    include/kipepeo/video/av1_encoder.h
    include/kipepeo/video/av1_decoder.h
    include/kipepeo/video/video_compressor.h
    include/kipepeo/video/rate_control.h
    include/kipepeo/video/types.h
    include/kipepeo/video/kip_mode_lowband.h
    include/kipepeo/video/rav1e_integration.h
)

set(VIDEO_LIBS "")

# iOS VideoToolbox support
if(APPLE)
    list(APPEND VIDEO_SOURCES ios/video_toolbox_decoder.mm)
    
    find_library(VIDEOTOOLBOX_LIBRARY VideoToolbox)
    find_library(COREMEDIA_LIBRARY CoreMedia)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    
    list(APPEND VIDEO_LIBS ${VIDEOTOOLBOX_LIBRARY} ${COREMEDIA_LIBRARY} ${COREVIDEO_LIBRARY})
endif()

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
    ${VIDEO_LIBS}
)

# Link third-party dependencies when available
# Note: rav1e is a Rust project that needs to be built separately
# and linked via its C API
if(TARGET rav1e)
    target_link_libraries(kipepeo_video PRIVATE rav1e)
endif()

# AV1 Decoder options: rav1d (Rust) or dav1d (C)
# rav1d is a Rust port of dav1d with C API compatibility
# dav1d is the original C implementation (uses Meson build system)
# Preference: Use rav1d if available, fallback to dav1d
if(TARGET rav1d OR RAV1D_FOUND)
    # Link rav1d library when available (built via Cargo)
    # The library will be linked via find_library or manual path
    message(STATUS "rav1d will be linked to kipepeo_video")
    target_compile_definitions(kipepeo_video PRIVATE KIPEPEO_USE_RAV1D)
elseif(TARGET dav1d OR DAV1D_FOUND)
    # Fallback to dav1d (original C implementation)
    # dav1d needs to be built with Meson and linked as a static library
    message(STATUS "dav1d will be linked to kipepeo_video")
    target_compile_definitions(kipepeo_video PRIVATE KIPEPEO_USE_DAV1D)
endif()

# Compile definitions
target_compile_definitions(kipepeo_video PRIVATE
    KIPEPEO_VIDEO_BUILD
)
