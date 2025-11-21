#include "native_interface.h"
#include "kipepeo/llm/llm_engine.h"
#include "kipepeo/video/video_compressor.h"
#include <string>
#include <cstring>

using namespace kipepeo;

// Helper function to convert jstring to std::string
std::string jstring_to_string(JNIEnv *env, jstring jstr) {
    if (!jstr) return "";
    const char *cstr = env->GetStringUTFChars(jstr, nullptr);
    std::string result(cstr);
    env->ReleaseStringUTFChars(jstr, cstr);
    return result;
}

// Helper function to convert std::string to jstring
jstring string_to_jstring(JNIEnv *env, const std::string &str) {
    return env->NewStringUTF(str.c_str());
}

// LLM Engine JNI Functions

JNIEXPORT jlong JNICALL
Java_com_kipepeo_KipepeoNative_initLLMEngine(JNIEnv *env, jobject thiz, jstring modelPath) {
    std::string path = jstring_to_string(env, modelPath);
    llm::LLMEngine *engine = new llm::LLMEngine();
    if (engine->initialize(path.c_str())) {
        return reinterpret_cast<jlong>(engine);
    }
    delete engine;
    return 0;
}

JNIEXPORT jstring JNICALL
Java_com_kipepeo_KipepeoNative_generateText(JNIEnv *env, jobject thiz, jlong engineHandle, jstring prompt) {
    llm::LLMEngine *engine = reinterpret_cast<llm::LLMEngine *>(engineHandle);
    if (!engine) {
        return string_to_jstring(env, "");
    }
    
    std::string prompt_str = jstring_to_string(env, prompt);
    char output[4096] = {0};
    
    if (engine->generate(prompt_str.c_str(), output, sizeof(output))) {
        return string_to_jstring(env, output);
    }
    
    return string_to_jstring(env, "");
}

JNIEXPORT jfloat JNICALL
Java_com_kipepeo_KipepeoNative_getTokensPerSecond(JNIEnv *env, jobject thiz, jlong engineHandle) {
    llm::LLMEngine *engine = reinterpret_cast<llm::LLMEngine *>(engineHandle);
    if (!engine) {
        return 0.0f;
    }
    return engine->get_tokens_per_second();
}

JNIEXPORT void JNICALL
Java_com_kipepeo_KipepeoNative_destroyLLMEngine(JNIEnv *env, jobject thiz, jlong engineHandle) {
    llm::LLMEngine *engine = reinterpret_cast<llm::LLMEngine *>(engineHandle);
    if (engine) {
        delete engine;
    }
}

// Video Compressor JNI Functions

JNIEXPORT jlong JNICALL
Java_com_kipepeo_KipepeoNative_initVideoCompressor(JNIEnv *env, jobject thiz) {
    video::VideoCompressor *compressor = new video::VideoCompressor();
    return reinterpret_cast<jlong>(compressor);
}

JNIEXPORT jbyteArray JNICALL
Java_com_kipepeo_KipepeoNative_compressFrame(JNIEnv *env, jobject thiz, jlong compressorHandle,
                                             jbyteArray inputData, jint width, jint height, jint format) {
    video::VideoCompressor *compressor = reinterpret_cast<video::VideoCompressor *>(compressorHandle);
    if (!compressor) {
        return nullptr;
    }
    
    jsize inputSize = env->GetArrayLength(inputData);
    jbyte *inputBytes = env->GetByteArrayElements(inputData, nullptr);
    
    // Allocate output buffer (assume 50% compression)
    size_t outputSize = inputSize / 2;
    uint8_t *outputBuffer = new uint8_t[outputSize];
    
    // Compress frame
    size_t actualOutputSize = outputSize;
    bool success = compressor->compress_frame(
        reinterpret_cast<const uint8_t *>(inputBytes),
        static_cast<size_t>(inputSize),
        outputBuffer,
        &actualOutputSize,
        width,
        height,
        format
    );
    
    env->ReleaseByteArrayElements(inputData, inputBytes, JNI_ABORT);
    
    if (!success) {
        delete[] outputBuffer;
        return nullptr;
    }
    
    // Create Java byte array
    jbyteArray result = env->NewByteArray(static_cast<jsize>(actualOutputSize));
    env->SetByteArrayRegion(result, 0, static_cast<jsize>(actualOutputSize),
                           reinterpret_cast<jbyte *>(outputBuffer));
    
    delete[] outputBuffer;
    return result;
}

JNIEXPORT jfloat JNICALL
Java_com_kipepeo_KipepeoNative_getCompressionRatio(JNIEnv *env, jobject thiz, jlong compressorHandle) {
    video::VideoCompressor *compressor = reinterpret_cast<video::VideoCompressor *>(compressorHandle);
    if (!compressor) {
        return 1.0f;
    }
    return compressor->get_compression_ratio();
}

JNIEXPORT void JNICALL
Java_com_kipepeo_KipepeoNative_destroyVideoCompressor(JNIEnv *env, jobject thiz, jlong compressorHandle) {
    video::VideoCompressor *compressor = reinterpret_cast<video::VideoCompressor *>(compressorHandle);
    if (compressor) {
        delete compressor;
    }
}

