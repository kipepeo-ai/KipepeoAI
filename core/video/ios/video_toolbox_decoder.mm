#include "video_toolbox_decoder.h"
#import <VideoToolbox/VideoToolbox.h>
#import <CoreMedia/CoreMedia.h>

struct VideoToolboxContext {
    VTDecompressionSessionRef session = nullptr;
    CMVideoFormatDescriptionRef formatDesc = nullptr;
};

// Callback for decompression output
void decompressionOutputCallback(void *decompressionOutputRefCon,
                                 void *sourceFrameRefCon,
                                 OSStatus status,
                                 VTDecodeInfoFlags infoFlags,
                                 CVImageBufferRef imageBuffer,
                                 CMTime presentationTimeStamp,
                                 CMTime presentationDuration) {
    if (status != noErr) {
        // Handle error
        return;
    }
    
    if (imageBuffer) {
        // In a real app, we would send this CVPixelBuffer to the rendering pipeline (Metal or OpenGL)
        // For now, we just acknowledge the frame was decoded.
    }
}

VideoToolboxDecoder::VideoToolboxDecoder() : ctx(new VideoToolboxContext()) {}

VideoToolboxDecoder::~VideoToolboxDecoder() {
    if (ctx->session) {
        VTDecompressionSessionInvalidate(ctx->session);
        CFRelease(ctx->session);
    }
    if (ctx->formatDesc) {
        CFRelease(ctx->formatDesc);
    }
    delete ctx;
}

bool VideoToolboxDecoder::init(int width, int height, const std::vector<uint8_t>& extraData) {
    // 1. Create Format Description
    // For AV1, we need to parse the config record. For simplicity, assuming H.264/HEVC style or pre-parsed.
    // Real implementation needs to handle 'av01' atom.
    
    OSStatus status = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                                          0, nullptr, 4, // Dummy params
                                                                          (const uint8_t *const*)&extraData, // Simplified
                                                                          &ctx->formatDesc);
    
    // Note: AV1 support in VideoToolbox requires iOS 17+ and A17 Pro or M3 usually.
    // We should check for kCMVideoCodecType_AV1 if available in the SDK.
    
    if (status != noErr) return false;
    
    // 2. Create Decompression Session
    VTDecompressionOutputCallbackRecord callbackRecord;
    callbackRecord.decompressionOutputCallback = decompressionOutputCallback;
    callbackRecord.decompressionOutputRefCon = this;
    
    CFDictionaryRef destinationImageBufferAttributes = nullptr; 
    // We might want kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange
    
    status = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                          ctx->formatDesc,
                                          nullptr,
                                          destinationImageBufferAttributes,
                                          &callbackRecord,
                                          &ctx->session);
                                          
    return (status == noErr);
}

bool VideoToolboxDecoder::decode(const uint8_t* data, size_t size, int64_t pts) {
    if (!ctx->session) return false;
    
    CMBlockBufferRef blockBuffer = nullptr;
    OSStatus status = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                                         (void*)data,
                                                         size,
                                                         kCFAllocatorNull,
                                                         nullptr,
                                                         0,
                                                         size,
                                                         0,
                                                         &blockBuffer);
                                                         
    if (status != noErr) return false;
    
    CMSampleBufferRef sampleBuffer = nullptr;
    // Create Sample Buffer... (Simplified, requires timing info)
    
    // VTDecompressionSessionDecodeFrame(ctx->session, sampleBuffer, 0, nullptr, nullptr);
    
    if (blockBuffer) CFRelease(blockBuffer);
    // if (sampleBuffer) CFRelease(sampleBuffer);
    
    return true;
}

void VideoToolboxDecoder::flush() {
    // VTDecompressionSessionWaitForAsynchronousFrames(ctx->session);
}
