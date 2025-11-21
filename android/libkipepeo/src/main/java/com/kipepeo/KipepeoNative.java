package com.kipepeo;

/**
 * JNI wrapper for Kipepeo native library
 * Provides Java interface to C++ LLM and video compression functionality
 */
public class KipepeoNative {
    static {
        System.loadLibrary("kipepeo");
    }
    
    // LLM Engine methods
    public native long initLLMEngine(String modelPath);
    public native String generateText(long engineHandle, String prompt);
    public native float getTokensPerSecond(long engineHandle);
    public native void destroyLLMEngine(long engineHandle);
    
    // Video Compressor methods
    public native long initVideoCompressor();
    public native byte[] compressFrame(long compressorHandle, byte[] inputData, int width, int height, int format);
    public native float getCompressionRatio(long compressorHandle);
    public native void destroyVideoCompressor(long compressorHandle);
}

