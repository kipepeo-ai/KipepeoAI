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
    include/kipepeo/video/rav1e_capi.h
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
# For now, we use a wrapper header (rav1e_capi.h) that declares the C API
# The actual rav1e library should be built and linked separately
# Example: find_library(RAV1E_LIBRARY NAMES rav1e librav1e)
# if(RAV1E_LIBRARY)
#     target_link_libraries(kipepeo_video PRIVATE ${RAV1E_LIBRARY})
#     target_compile_definitions(kipepeo_video PRIVATE KIPEPEO_USE_RAV1E)
# endif()

# Include directories for third-party libraries
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/rav1d/src/include")
    target_include_directories(kipepeo_video PRIVATE
        "${CMAKE_SOURCE_DIR}/third_party/rav1d/src/include"
    )
elseif(EXISTS "${CMAKE_SOURCE_DIR}/third_party/dav1d/include")
    target_include_directories(kipepeo_video PRIVATE
        "${CMAKE_SOURCE_DIR}/third_party/dav1d/include"
    )
endif()

# AV1 Decoder options: rav1d (Rust) or dav1d (C)
# rav1d is a Rust port of dav1d with C API compatibility
# dav1d is the original C implementation (uses Meson build system)
# Preference: Use rav1d if available, fallback to dav1d
# 
# Note: Libraries should be built separately and linked
# Example for rav1d: cargo build --release (produces librav1e.a or .so)
# Example for dav1d: meson build && ninja -C build (produces libdav1d.a)
find_library(RAV1D_LIBRARY NAMES rav1d librav1d
    PATHS "${CMAKE_SOURCE_DIR}/third_party/rav1d/target/release"
          "${CMAKE_SOURCE_DIR}/third_party/rav1d/target/debug"
    NO_DEFAULT_PATH
)
find_library(DAV1D_LIBRARY NAMES dav1d libdav1d
    PATHS "${CMAKE_SOURCE_DIR}/third_party/dav1d/build"
    NO_DEFAULT_PATH
)

if(RAV1D_LIBRARY)
    target_link_libraries(kipepeo_video PRIVATE ${RAV1D_LIBRARY})
    target_compile_definitions(kipepeo_video PRIVATE KIPEPEO_USE_RAV1D)
    message(STATUS "Found rav1d: ${RAV1D_LIBRARY}")
elseif(DAV1D_LIBRARY)
    target_link_libraries(kipepeo_video PRIVATE ${DAV1D_LIBRARY})
    target_compile_definitions(kipepeo_video PRIVATE KIPEPEO_USE_DAV1D)
    message(STATUS "Found dav1d: ${DAV1D_LIBRARY}")
else()
    message(WARNING "Neither rav1d nor dav1d found. Decoder will not be available.")
    message(STATUS "  To build rav1d: cd third_party/rav1d && cargo build --release")
    message(STATUS "  To build dav1d: cd third_party/dav1d && meson build && ninja -C build")
endif()

# Find rav1e library (encoder)
find_library(RAV1E_LIBRARY NAMES rav1e librav1e
    PATHS "${CMAKE_SOURCE_DIR}/third_party/rav1e/target/release"
          "${CMAKE_SOURCE_DIR}/third_party/rav1e/target/debug"
    NO_DEFAULT_PATH
)

if(RAV1E_LIBRARY)
    target_link_libraries(kipepeo_video PRIVATE ${RAV1E_LIBRARY})
    target_compile_definitions(kipepeo_video PRIVATE KIPEPEO_USE_RAV1E)
    message(STATUS "Found rav1e: ${RAV1E_LIBRARY}")
else()
    message(WARNING "rav1e not found. Encoder will not be available.")
    message(STATUS "  To build rav1e: cd third_party/rav1e && cargo build --release --features capi")
endif()

# Compile definitions
target_compile_definitions(kipepeo_video PRIVATE
    KIPEPEO_VIDEO_BUILD
)
