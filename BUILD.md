# Build System and CI/CD Setup Guide

This document provides comprehensive instructions for the Kipepeo build system and CI/CD infrastructure.

## Overview

Kipepeo uses a modern CMake-based build system with:
- **CMake 3.22+** for C/C++ core libraries
- **Android Gradle Plugin 8.7+** for Android builds
- **GitHub Actions** for CI/CD automation
- **Firebase App Distribution** for beta testing

## Build System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Root CMakeLists.txt                     │
│  Configures global options, compiler flags, Android NDK     │
└────────────┬────────────────────────────────────────────────┘
             │
             ├──> core/CMakeLists.txt
             │    ├──> core/kernels/CMakeLists.txt → kipepeo_kernels.a
             │    ├──> core/quantization/CMakeLists.txt → kipepeo_quantization.a
             │    ├──> core/llm/CMakeLists.txt → kipepeo_llm.a
             │    └──> core/video/CMakeLists.txt → kipepeo_video.a
             │
             ├──> android/libkipepeo/CMakeLists.txt
             │    └──> Builds libkipepeo.so (links all core/*.a)
             │
             └──> tests/CMakeLists.txt
                  └──> Unit and integration tests
```

## Local Build Instructions

### Command Line Build (Gradle)

```bash
# Debug build
cd android
./gradlew assembleDebug

# Release build
./gradlew assembleRelease

# Clean build
./gradlew clean assembleRelease

# Build with specific ABI
./gradlew assembleRelease -PABI=arm64-v8a
```

### Android Studio Build

1. Open `android/` directory in Android Studio
2. Wait for Gradle sync
3. Select build variant (debug/release)
4. Build → Make Project (Ctrl+F9)
5. APK location: `android/app/build/outputs/apk/`

### CMake-Only Build (Core Libraries)

```bash
# Configure
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DKIPEPEO_BUILD_LLM=ON \
  -DKIPEPEO_BUILD_VIDEO=ON \
  -DKIPEPEO_BUILD_KERNELS=ON \
  -DKIPEPEO_BUILD_QUANTIZATION=ON

# Build
cmake --build . --parallel $(nproc)

# Test
ctest --output-on-failure
```

### Android NDK Build (Advanced)

```bash
mkdir build-android && cd build-android
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-21 \
  -DANDROID_STL=c++_shared \
  -DKIPEPEO_BUILD_ANDROID=ON
  
make -j$(nproc)
```

## Release Signing

### Generate Keystore (First Time)

```bash
cd android
keytool -genkey -v \
  -keystore release.keystore \
  -alias kipepeo-release-key \
  -keyalg RSA \
  -keysize 2048 \
  -validity 10000
```

### Configure Signing

1. Copy template: `cp keystore.properties.example keystore.properties`
2. Edit `keystore.properties` with your values:
   ```properties
   storeFile=release.keystore
   storePassword=YOUR_PASSWORD
   keyAlias=kipepeo-release-key
   keyPassword=YOUR_KEY_PASSWORD
   ```
3. **IMPORTANT:** Add `keystore.properties` and `*.keystore` to `.gitignore`

## CI/CD with GitHub Actions

### Workflows

1. **`android-build.yml`** - Build APK on every push
   - Builds debug and release APKs
   - Uploads artifacts
   - Deploys to Firebase (main branch only)

2. **`tests.yml`** - Run C++ tests
   - Tests on x64 runners
   - Tests on ARM (via QEMU)
   - Generates test reports

### Setup GitHub Secrets

Required secrets for CI/CD:

```
FIREBASE_APP_ID=1:123456789:android:abcdef
FIREBASE_SERVICE_ACCOUNT=<JSON content>
```

To get these:
1. Go to Firebase Console → Project Settings
2. Download service account JSON
3. Copy app ID from Firebase console
4. Add both to GitHub repo secrets

### Manual Workflow Trigger

```bash
# Trigger via GitHub CLI
gh workflow run android-build.yml

# Trigger via web
# Go to Actions tab → Select workflow → Run workflow
```

## Model Downloads

### Automatic (Recommended)

Models are automatically downloaded to `android/app/src/main/assets/models/` during first app launch.

### Manual Pre-download

```bash
cd tools/model-downloader
pip install -r requirements.txt

# Download 34B model (recommended)
python download_models.py --models 34B

# Download all models
python download_models.py --all

# Dry run (preview)
python download_models.py --models 7B --dry-run
```

## Troubleshooting

### CMake Errors

**Error:** `Could not find toolchain file`
```bash
# Set ANDROID_NDK environment variable
export ANDROID_NDK=/path/to/ndk
# Or specify directly
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/ndk/build/cmake/android.toolchain.cmake
```

**Error:** `Target "kipepeo_llm" not found`
```bash
# Build core libraries first
cd core
cmake --build .
```

### Gradle Build Errors

**Error:** `NDK not found`
```bash
# Install NDK via SDK Manager
sdkmanager --install "ndk;25.2.9519653"
```

**Error:** `Execution failed for task ':app:mergeReleaseNativeLibs'`
```bash
# Clean and rebuild
./gradlew clean
./gradlew assembleRelease --stacktrace
```

### APK Installation Errors

**Error:** `INSTALL_FAILED_NO_MATCHING_ABIS`
- Your device doesn't support arm64-v8a
- Solution: Build with armeabi-v7a ABI (edit `app/build.gradle`)

**Error:** `INSTALL_PARSE_FAILED_NO_CERTIFICATES`
- APK not signed properly
- Solution: Configure release signing or use debug build

## Build Performance Optimization

### Gradle

```properties
# android/gradle.properties
org.gradle.jvmargs=-Xmx8192m
org.gradle.parallel=true
org.gradle.caching=true
```

### CMake

```bash
# Use ninja instead of make
cmake .. -GNinja

# Enable ccache
cmake .. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

## Firebase App Distribution

### Initial Setup

1. Install Firebase CLI:
   ```bash
   npm install -g firebase-tools
   firebase login
   ```

2. Initialize Firebase in project:
   ```bash
   cd android
   firebase init
   # Select: App Distribution
   ```

3. Create tester group "kenya-testers"

### Manual Deploy

```bash
firebase appdistribution:distribute \
  app/build/outputs/apk/release/app-release.apk \
  --app YOUR_FIREBASE_APP_ID \
  --groups kenya-testers \
  --release-notes "Manual deployment of Kipepeo v1.0.0"
```

### Automated Deploy (via GitHub Actions)

Automatically deploys on push to `main` branch. See `.github/workflows/android-build.yml`.

## Version Tagging

### Create Release Tag

```bash
# Ensure everything is committed
git add .
git commit -m "[BUILD] Release v1.0.0-kenya"

# Create annotated tag
git tag -a v1.0.0-kenya -m "Kipepeo AI v1.0.0 - Kenya Release

Features:
- Offline LLM inference (7B-70B)
- Video compression (40-60% savings)
- ARM NEON optimizations
- Jetpack Compose UI

Vipengele:
- AI ya nje ya mtandao
- Okoa data kwa video (40-60%)
- Haraka kwa simu za MediaTek
- UI ya Kiswahili/English/Sheng
"

# Push tag
git push origin v1.0.0-kenya
```

### GitHub Release

1. Go to Releases → Draft a new release
2. Select tag: `v1.0.0-kenya`
3. Upload `RELEASE_NOTES.md`
4. Attach APK file
5. Publish release

## Commit Prefixes

Use these prefixes for commit messages:
- `[BUILD]` - Build system changes
- `[CI]` - CI/CD configuration
- `[CORE]` - Core C++ library changes
- `[ANDROID]` - Android app changes
- `[DOCS]` - Documentation
- `[FIX]` - Bug fixes

Example:
```bash
git commit -m "[BUILD] Add GitHub Actions workflow for APK builds"
```

## Additional Resources

- CMake Documentation: https://cmake.org/documentation/
- Android NDK Guide: https://developer.android.com/ndk/guides
- GitHub Actions: https://docs.github.com/en/actions
- Firebase App Distribution: https://firebase.google.com/docs/app-distribution
