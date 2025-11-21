#include <jni.h>
#include "native_interface.h"
#include <android/log.h>
#include <string.h>

#define TAG "KipepeoJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// JNI OnLoad - called when library is loaded
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("Kipepeo native library loaded");
    
    // Initialize Kipepeo
    if (!kipepeo_init()) {
        LOGE("Failed to initialize Kipepeo");
        return JNI_ERR;
    }
    
    return JNI_VERSION_1_6;
}

// JNI OnUnload - called when library is unloaded
JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGI("Kipepeo native library unloading");
    kipepeo_shutdown();
}

// Native method: activateKipepeoEngine
extern "C" JNIEXPORT jboolean JNICALL
Java_ai_kipepeo_native_NativeBridge_activateKipepeoEngine(JNIEnv* env, jobject /* this */) {
    LOGI("JNI: activateKipepeoEngine called");
    return kipepeo_activate_engine() ? JNI_TRUE : JNI_FALSE;
}

// Native method: deactivateKipepeoEngine
extern "C" JNIEXPORT void JNICALL
Java_ai_kipepeo_native_NativeBridge_deactivateKipepeoEngine(JNIEnv* env, jobject /* this */) {
    LOGI("JNI: deactivateKipepeoEngine called");
    kipepeo_deactivate_engine();
}

// Native method: getTokensPerSecond
extern "C" JNIEXPORT jfloat JNICALL
Java_ai_kipepeo_native_NativeBridge_getTokensPerSecond(JNIEnv* env, jobject /* this */) {
    return (jfloat)kipepeo_get_tokens_per_second();
}

// Native method: getDataSaved
extern "C" JNIEXPORT jlong JNICALL
Java_ai_kipepeo_native_NativeBridge_getDataSaved(JNIEnv* env, jobject /* this */) {
    return (jlong)kipepeo_get_data_saved();
}

// Native method: isRootAvailable
extern "C" JNIEXPORT jboolean JNICALL
Java_ai_kipepeo_native_NativeBridge_isRootAvailable(JNIEnv* env, jobject /* this */) {
    return kipepeo_is_root_available() ? JNI_TRUE : JNI_FALSE;
}

// Native method: getHookStatus
extern "C" JNIEXPORT jstring JNICALL
Java_ai_kipepeo_native_NativeBridge_getHookStatus(JNIEnv* env, jobject /* this */) {
    const char* status = kipepeo_get_hook_status();
    return env->NewStringUTF(status);
}

// Native method: getCompressionRatio
extern "C" JNIEXPORT jdouble JNICALL
Java_ai_kipepeo_native_NativeBridge_getCompressionRatio(JNIEnv* env, jobject /* this */) {
    return (jdouble)kipepeo_get_compression_ratio();
}

// Native method: resetStats
extern "C" JNIEXPORT void JNICALL
Java_ai_kipepeo_native_NativeBridge_resetStats(JNIEnv* env, jobject /* this */) {
    LOGI("JNI: resetStats called");
    kipepeo_reset_stats();
}
