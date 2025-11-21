#include <jni.h>
#include <string>
#include "native_interface.h"
#include "voice_engine.h"
#include "vision_engine.h"
#include "health_engine.h"
#include "tutor_engine.h"

using namespace kipepeo::voice;
using namespace kipepeo::vision;
using namespace kipepeo::health;
using namespace kipepeo::education;

extern "C" {

// --- Health JNI ---

JNIEXPORT void JNICALL
Java_com_kipepeo_app_health_HealthViewModel_initHealthEngine(JNIEnv *env, jobject thiz) {
    HealthEngine::instance().init();
}

JNIEXPORT jstring JNICALL
Java_com_kipepeo_app_health_HealthViewModel_diagnoseNative(JNIEnv *env, jobject thiz, jstring symptoms) {
    const char *symptomsChars = env->GetStringUTFChars(symptoms, nullptr);
    Diagnosis d = HealthEngine::instance().diagnose(std::string(symptomsChars));
    env->ReleaseStringUTFChars(symptoms, symptomsChars);
    
    std::string result = d.condition + "|" + std::to_string(d.confidence) + "|" + d.recommendation + "|" + d.nearestClinic;
    return env->NewStringUTF(result.c_str());
}

// --- Education JNI ---

JNIEXPORT void JNICALL
Java_com_kipepeo_app_learn_LearnViewModel_initTutorEngine(JNIEnv *env, jobject thiz) {
    TutorEngine::instance().init();
}

JNIEXPORT jstring JNICALL
Java_com_kipepeo_app_learn_LearnViewModel_askTutorNative(JNIEnv *env, jobject thiz, jstring subject, jstring question) {
    const char *subjectChars = env->GetStringUTFChars(subject, nullptr);
    const char *questionChars = env->GetStringUTFChars(question, nullptr);
    
    TutorResponse r = TutorEngine::instance().ask(std::string(subjectChars), std::string(questionChars));
    
    env->ReleaseStringUTFChars(subject, subjectChars);
    env->ReleaseStringUTFChars(question, questionChars);
    
    return env->NewStringUTF(r.answer.c_str());
}

// --- Finance JNI ---

#include "finance_engine.h"
using namespace kipepeo::finance;

JNIEXPORT void JNICALL
Java_com_kipepeo_app_money_MoneyViewModel_initFinanceEngine(JNIEnv *env, jobject thiz) {
    FinanceEngine::instance().init();
}

JNIEXPORT jstring JNICALL
Java_com_kipepeo_app_money_MoneyViewModel_analyzeTransactionsNative(JNIEnv *env, jobject thiz, jstring smsData) {
    const char *smsChars = env->GetStringUTFChars(smsData, nullptr);
    auto txs = FinanceEngine::instance().analyzeTransactions(std::string(smsChars));
    env->ReleaseStringUTFChars(smsData, smsChars);
    
    std::string result;
    for (const auto& tx : txs) {
        result += tx.id + "|" + tx.type + "|" + std::to_string(tx.amount) + "|" + tx.date + "|" + (tx.isSuspicious ? "1" : "0") + ";";
    }
    return env->NewStringUTF(result.c_str());
}

// --- Mesh JNI ---

#include "mesh_engine.h"
using namespace kipepeo::mesh;

JNIEXPORT void JNICALL
Java_com_kipepeo_app_mesh_MeshViewModel_initMeshEngine(JNIEnv *env, jobject thiz) {
    MeshEngine::instance().init();
}

JNIEXPORT void JNICALL
Java_com_kipepeo_app_mesh_MeshViewModel_startDiscoveryNative(JNIEnv *env, jobject thiz) {
    MeshEngine::instance().startDiscovery();
}

JNIEXPORT jstring JNICALL
Java_com_kipepeo_app_mesh_MeshViewModel_getPeersNative(JNIEnv *env, jobject thiz) {
    auto peers = MeshEngine::instance().getPeers();
    
    std::string result;
    for (const auto& peer : peers) {
        result += peer.id + "|" + peer.name + "|" + std::to_string(peer.signalStrength) + "|" + (peer.isConnected ? "1" : "0") + ";";
    }
    return env->NewStringUTF(result.c_str());
}

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
