#include <jni.h>
#include <string>
#include "native_interface.h"
#include "voice_engine.h"
#include "vision_engine.h"

using namespace kipepeo::voice;
using namespace kipepeo::vision;

extern "C" {

// --- Vision JNI ---

JNIEXPORT void JNICALL
Java_com_kipepeo_app_vision_VisionViewModel_initVisionEngine(JNIEnv *env, jobject thiz) {
    VisionEngine::instance().init();
}

JNIEXPORT jstring JNICALL
Java_com_kipepeo_app_vision_VisionViewModel_describeImageNative(JNIEnv *env, jobject thiz, jbyteArray imageData) {
    jsize len = env->GetArrayLength(imageData);
    jbyte *body = env->GetByteArrayElements(imageData, 0);
    
    std::vector<uint8_t> data(body, body + len);
    std::string result = VisionEngine::instance().describeImage(data);
    
    env->ReleaseByteArrayElements(imageData, body, 0);
    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jbyteArray JNICALL
Java_com_kipepeo_app_vision_VisionViewModel_generateImageNative(JNIEnv *env, jobject thiz, jstring prompt) {
    const char *promptChars = env->GetStringUTFChars(prompt, nullptr);
    std::vector<uint8_t> result = VisionEngine::instance().generateImage(std::string(promptChars));
    env->ReleaseStringUTFChars(prompt, promptChars);
    
    jbyteArray ret = env->NewByteArray(result.size());
    env->SetByteArrayRegion(ret, 0, result.size(), reinterpret_cast<const jbyte*>(result.data()));
    return ret;
}

// --- Voice JNI ---

JNIEXPORT void JNICALL
Java_com_kipepeo_app_calls_CallViewModel_initVoiceEngine(JNIEnv *env, jobject thiz) {
    VoiceEngine::instance().init();
}

JNIEXPORT void JNICALL
Java_com_kipepeo_app_calls_CallViewModel_startCallNative(JNIEnv *env, jobject thiz, jstring peerId) {
    const char *peerIdChars = env->GetStringUTFChars(peerId, nullptr);
    VoiceEngine::instance().startCall(std::string(peerIdChars));
    env->ReleaseStringUTFChars(peerId, peerIdChars);
}

JNIEXPORT void JNICALL
Java_com_kipepeo_app_calls_CallViewModel_endCallNative(JNIEnv *env, jobject thiz) {
    VoiceEngine::instance().endCall();
}

// JNI_OnLoad - called when the library is loaded
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
}

} // extern "C"
