# API Reference

## C++ API

### LLM Engine

#### `kipepeo::llm::LLMEngine`

Main interface for LLM inference.

```cpp
#include "kipepeo/llm/llm_engine.h"

// Initialize engine
kipepeo::llm::LLMEngine engine;
engine.initialize("/path/to/model.gguf");

// Generate text
char output[4096];
engine.generate("Habari yako?", output, sizeof(output));

// Get performance
float tokens_per_sec = engine.get_tokens_per_second();
```

### Video Compressor

#### `kipepeo::video::VideoCompressor`

Main interface for video compression.

```cpp
#include "kipepeo/video/video_compressor.h"

// Initialize compressor
kipepeo::video::VideoCompressor compressor;

// Compress frame
uint8_t* output_data;
size_t output_size;
compressor.compress_frame(input_data, input_size,
                         output_data, &output_size,
                         width, height, format);

// Get compression ratio
float ratio = compressor.get_compression_ratio();
```

## Java/Android API

### `com.kipepeo.KipepeoNative`

JNI wrapper for native functionality.

```java
import com.kipepeo.KipepeoNative;

// Initialize LLM engine
KipepeoNative native = new KipepeoNative();
long engineHandle = native.initLLMEngine("/path/to/model.gguf");

// Generate text
String prompt = "Habari yako?";
String response = native.generateText(engineHandle, prompt);

// Get performance
float tokensPerSec = native.getTokensPerSecond(engineHandle);

// Cleanup
native.destroyLLMEngine(engineHandle);
```

## Detailed API Documentation

Full API documentation will be generated as development progresses.

