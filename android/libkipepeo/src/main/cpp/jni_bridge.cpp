#include <jni.h>
#include "native_interface.h"

// JNI_OnLoad - called when the library is loaded
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    
    // Initialize any global state here
    // TODO: Add initialization code if needed
    
    return JNI_VERSION_1_6;
}

// JNI_OnUnload - called when the library is unloaded
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    // Cleanup any global state here
    // TODO: Add cleanup code if needed
}

