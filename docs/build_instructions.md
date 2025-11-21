# Build Instructions

## Prerequisites

### Required Software

1. **CMake** 3.22 or later
   ```bash
   # Ubuntu/Debian
   sudo apt-get install cmake
   
   # macOS
   brew install cmake
   
   # Windows
   # Download from https://cmake.org/download/
   ```

2. **Android NDK** r25c or later
   - Download from: https://developer.android.com/ndk/downloads
   - Set `ANDROID_NDK` environment variable:
     ```bash
     export ANDROID_NDK=/path/to/android-ndk-r25c
     ```

3. **Android SDK** (API 33+)
   - Install via Android Studio
   - Set `ANDROID_SDK` environment variable:
     ```bash
     export ANDROID_SDK=/path/to/android-sdk
     ```

4. **C/C++ Compiler**
   - Linux: GCC or Clang
   - macOS: Xcode Command Line Tools
   - Windows: Visual Studio or MinGW

5. **Python** 3.8+ (for quantization tools)
   ```bash
   python3 --version
   ```

## Desktop Build

### Linux/macOS

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS
```

### Windows

```bash
# Create build directory
mkdir build
cd build

# Configure (using Visual Studio)
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release
```

## Android Build

### Using CMake with Android NDK

```bash
# Create build directory
mkdir build-android
cd build-android

# Configure
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-33 \
    -DANDROID_STL=c++_shared \
    -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)
```

### Using Android Studio

1. Open Android Studio
2. Open the `android/` directory
3. Sync Gradle files (File > Sync Project with Gradle Files)
4. Build the project (Build > Make Project)

The native library will be built automatically via CMake integration.

## Build Options

Configure build options via CMake:

```bash
cmake .. \
    -DKIPEPEO_BUILD_LLM=ON \
    -DKIPEPEO_BUILD_VIDEO=ON \
    -DKIPEPEO_BUILD_KERNELS=ON \
    -DKIPEPEO_BUILD_QUANTIZATION=ON \
    -DKIPEPEO_BUILD_ANDROID=ON \
    -DKIPEPEO_BUILD_TESTS=OFF
```

## Troubleshooting

### CMake not finding Android NDK

Set the `ANDROID_NDK` environment variable:
```bash
export ANDROID_NDK=/path/to/android-ndk-r25c
```

### Build errors on Android

Ensure you're using the correct API level:
```bash
-DANDROID_PLATFORM=android-33
```

### Missing dependencies

Third-party dependencies need to be set up first. See `third_party/README.md`.

