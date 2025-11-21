# Kipepeo Android - NDK + PLT Hooking

This directory contains the Android application for Kipepeo AI with system-level video codec interception.

## Structure

```
android/
├── app/                          # Main Android app (Jetpack Compose UI)
│   └── src/main/
│       ├── kotlin/ai/kipepeo/   # Kotlin source code
│       │   ├── MainActivity.kt
│       │   ├── KipepeoViewModel.kt
│       │   ├── native/
│       │   │   └── NativeBridge.kt
│       │   └── ui/
│       │       ├── MainScreen.kt
│       │       └── theme/
│       ├── res/                 # Resources
│       └── AndroidManifest.xml
│
├── kipepeo-ndk/                 # Native library module
│   ├── src/main/cpp/
│   │   ├── hook_manager.cpp/.h       # PLT hooking manager
│   │   ├── mediacodec_hooks.cpp/.h   # MediaCodec interception
│   │   ├── data_tracker.cpp/.h       # Data usage tracking
│   │   ├── native_interface.cpp/.h   # C API
│   │   └── jni_bridge.cpp            # JNI methods
│   ├── CMakeLists.txt           # Native build configuration
│   └── build.gradle             # Module Gradle configuration
│
├── magisk/                      # Magisk module (optional, for root)
│   ├── module.prop
│   ├── install.sh
│   └── service.sh
│
├── build.gradle                 # Project Gradle configuration
└── settings.gradle              # Project settings
```

## Features

### 1. PLT Hooking (Root Mode)
- Intercepts `libstagefright.so` (Android MediaCodec)
- Intercepts `libmedia.so` (Media framework)  
- Intercepts `libavcodec.so` (FFmpeg in bundled apps)
- Redirects H.264/HEVC/VP8/VP9 → AV1
- Transparent to apps

### 2. Non-Root Fallback
- Graceful degradation on non-rooted devices
- VPN-based data tracking (limited functionality)
- App-level codec usage monitoring

### 3. Real-time Metrics
- LLM inference tokens/sec
- Data saved via AV1 compression
- Compression ratio tracking
- Live statistics in Beautiful Material3 UI

## Build Instructions

### Prerequisites
- Android Studio Hedgehog (2023.1.1) or later
- Android NDK 26.1.10909125
- CMake 3.22.1+
- Kotlin 1.9.22+

### Build Steps

1. **Clone with submodules** (if not already done):
   ```bash
   git submodule update --init --recursive
   ```

2. **Build native library**:
   ```bash
   cd android
   ./gradlew :kipepeo-ndk:assembleRelease
   ```

3. **Build APK**:
   ```bash
   ./gradlew :app:assembleDebug
   # or for release:
   ./gradlew :app:assembleRelease
   ```

4. **Install on device**:
   ```bash
   ./gradlew installDebug
   # or
   adb install -r app/build/outputs/apk/debug/app-debug.apk
   ```

## Testing

### Non-Root Testing
1. Install APK on device (Android 11-15)
2. Launch Kipepeo app
3. Tap "Activate Kipepeo Engine" switch
4. Expected: Warning shown (requires root for full functionality)
5. Open YouTube/WhatsApp, play video
6. Verify data counter updates

### Root Testing (Magisk)
1. Install Magisk module:
   ```bash
   adb push magisk/kipepeo-module.zip /sdcard/
   # Install via Magisk Manager
   ```
2. Reboot device
3. Install Kipepeo APK
4. Activate engine
5. Monitor logs:
   ```bash
   adb logcat | grep -E "Kipepeo|KipepeoHook|KipepeoCodec"
   ```
6. Verify MediaCodec hooks intercept calls
7. Check AV1 encoding is used

## Debugging

### View Native Logs
```bash
adb logcat -s KipepeoHook:* KipepeoCodec:* KipepeoData:* KipepeoNative:* KipepeoJNI:*
```

### Check Hook Status
The app UI displays hook status:
- "Active (Root Mode)" - PLT hooks working
- "Active (Non-Root Mode - Limited)" - Fallback mode
- "Failed" - Hooks failed to install
- "Inactive" - Engine not activated

## Troubleshooting

### "Failed to activate engine"
- **Cause**: PLT hooking requires root on Android 10+
- **Solution**: Install Magisk module or use non-root mode (limited functionality)

### "libkipepeo.so not found"
- **Cause**: Native library not built or wrong ABI
- **Solution**: Clean and rebuild: `./gradlew clean :kipepeo-ndk:assembleRelease`

### No data savings shown
- **Cause**: Hooks not intercepting codec calls
- **Solution**: Check logcat for hook registration, ensure root access

## Architecture

### Hook Flow
```
App calls MediaCodec.createEncoderByType("video/avc")
    ↓
PLT Hook intercepts (libstagefright.so)
    ↓
Kipepeo hook_manager redirects to AV1
    ↓
Returns MediaCodec for "video/av01"
    ↓
App encodes with AV1 (transparent)
```

### Data Tracking
```
Root mode: Read /proc/net/xt_qtaguid/stats
Non-root mode: VPNService packet capture (TODO)
    ↓
Calculate: original_size - compressed_size
    ↓
Update UI metrics via JNI callbacks
```

## Commit Prefix
All commits related to this Android implementation should use:
```
[ANDROID-HOOK] Your commit message
```

## License
Apache 2.0 - See root LICENSE file
