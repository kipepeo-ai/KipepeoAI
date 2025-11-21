#pragma once

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

// LLM functions
JNIEXPORT jlong JNICALL
Java_com_kipepeo_KipepeoNative_initLLMEngine(JNIEnv *env, jobject thiz, jstring modelPath);

JNIEXPORT jstring JNICALL
Java_com_kipepeo_KipepeoNative_generateText(JNIEnv *env, jobject thiz, jlong engineHandle, jstring prompt);

JNIEXPORT jfloat JNICALL
Java_com_kipepeo_KipepeoNative_getTokensPerSecond(JNIEnv *env, jobject thiz, jlong engineHandle);

JNIEXPORT void JNICALL
Java_com_kipepeo_KipepeoNative_destroyLLMEngine(JNIEnv *env, jobject thiz, jlong engineHandle);

// Video compression functions
JNIEXPORT jlong JNICALL
Java_com_kipepeo_KipepeoNative_initVideoCompressor(JNIEnv *env, jobject thiz);

JNIEXPORT jbyteArray JNICALL
Java_com_kipepeo_KipepeoNative_compressFrame(JNIEnv *env, jobject thiz, jlong compressorHandle,
                                             jbyteArray inputData, jint width, jint height, jint format);

JNIEXPORT jfloat JNICALL
Java_com_kipepeo_KipepeoNative_getCompressionRatio(JNIEnv *env, jobject thiz, jlong compressorHandle);

JNIEXPORT void JNICALL
Java_com_kipepeo_KipepeoNative_destroyVideoCompressor(JNIEnv *env, jobject thiz, jlong compressorHandle);

#ifdef __cplusplus
}
#endif

