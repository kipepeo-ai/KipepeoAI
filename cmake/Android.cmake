# Android NDK Toolchain Configuration for Kipepeo
# This file configures the Android NDK build environment

# Android API level configuration
if(NOT ANDROID_PLATFORM_LEVEL)
    set(ANDROID_PLATFORM_LEVEL 33 CACHE STRING "Android API level")
endif()

# Minimum API level
set(ANDROID_MIN_API_LEVEL 21)

# Validate API level
if(ANDROID_PLATFORM_LEVEL LESS ANDROID_MIN_API_LEVEL)
    message(FATAL_ERROR "Android API level must be at least ${ANDROID_MIN_API_LEVEL}, got ${ANDROID_PLATFORM_LEVEL}")
endif()

# ABI configuration - prioritize arm64-v8a, support armeabi-v7a
if(NOT ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "Android ABI")
    set_property(CACHE ANDROID_ABI PROPERTY STRINGS "arm64-v8a" "armeabi-v7a")
endif()

# Set Android platform
set(ANDROID_PLATFORM "android-${ANDROID_PLATFORM_LEVEL}")

# Compiler flags for Android
if(ANDROID_ABI STREQUAL "arm64-v8a")
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    set(CMAKE_ANDROID_ARCH_ABI "arm64-v8a")
    # Enable NEON for arm64 (always available)
    add_compile_definitions(KIPEPEO_NEON_ENABLED)
    add_compile_options(-march=armv8-a+fp+simd)
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(CMAKE_SYSTEM_PROCESSOR "armv7-a")
    set(CMAKE_ANDROID_ARCH_ABI "armeabi-v7a")
    # Enable NEON for armv7
    add_compile_definitions(KIPEPEO_NEON_ENABLED)
    add_compile_options(-march=armv7-a -mfpu=neon -mfloat-abi=softfp)
endif()

# MediaTek-specific optimizations
# MediaTek Helio G85/G99 optimizations
add_compile_definitions(
    KIPEPEO_TARGET_MEDIATEK
    KIPEPEO_OPTIMIZE_FOR_MOBILE
)

# Linker flags for Android
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined")

# Android-specific compile definitions
add_compile_definitions(
    ANDROID
    __ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
    KIPEPEO_ANDROID_BUILD
)

# Optimization flags for mobile devices
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Size optimization
    add_compile_options(-Os)
    # Link-time optimization
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# Log Android configuration
message(STATUS "Android Platform: ${ANDROID_PLATFORM}")
message(STATUS "Android ABI: ${ANDROID_ABI}")
message(STATUS "Android API Level: ${ANDROID_PLATFORM_LEVEL}")
message(STATUS "System Processor: ${CMAKE_SYSTEM_PROCESSOR}")

