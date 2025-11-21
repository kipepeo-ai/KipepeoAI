# Third-Party Dependencies

This directory contains external dependencies that will be integrated into Kipepeo.

## Dependencies

### llama.cpp
- **Purpose**: LLM inference engine base
- **Status**: To be forked and integrated
- **Fork Location**: https://github.com/ggerganov/llama.cpp
- **Integration Notes**: 
  - Will be forked and modified for custom quantization (1.58-bit, 1-bit AfricaQuant)
  - ARM NEON optimizations for MediaTek chips
  - Swahili/English/Sheng tokenizer support

### rav1e
- **Purpose**: AV1 encoder
- **Status**: To be forked and integrated
- **Fork Location**: https://github.com/xiph/rav1e
- **Integration Notes**:
  - Will be forked as "rav1e-africa"
  - Custom rate control for African content
  - Skin-tone optimization
  - Real-time encoding on MediaTek G99

### dav1d
- **Purpose**: AV1 decoder
- **Status**: To be forked and integrated
- **Fork Location**: https://github.com/videolan/dav1d
- **Integration Notes**:
  - Fast AV1 decoding for mobile devices
  - Optimized for low-end Android phones

### whisper.cpp
- **Purpose**: Speech recognition
- **Status**: To be forked and integrated
- **Fork Location**: https://github.com/ggerganov/whisper.cpp
- **Integration Notes**:
  - Offline speech-to-text
  - Swahili/English support
  - Voice interface for AI assistant

## Setup Instructions

### Using Git Submodules

```bash
# Add submodules
git submodule add https://github.com/ggerganov/llama.cpp third_party/llama.cpp
git submodule add https://github.com/xiph/rav1e third_party/rav1e
git submodule add https://github.com/videolan/dav1d third_party/dav1d
git submodule add https://github.com/ggerganov/whisper.cpp third_party/whisper.cpp

# Initialize and update
git submodule update --init --recursive
```

### Manual Fork Setup

1. Fork each repository to your organization
2. Clone the forks into this directory
3. Create integration branches for Kipepeo-specific modifications
4. Update CMakeLists.txt to reference the local paths

## Integration Status

- [ ] llama.cpp - Not yet integrated
- [ ] rav1e - Not yet integrated
- [ ] dav1d - Not yet integrated
- [ ] whisper.cpp - Not yet integrated

