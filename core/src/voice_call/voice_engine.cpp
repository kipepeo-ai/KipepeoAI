#include "voice_engine.h"
#include <iostream>

namespace kipepeo {
namespace voice {

VoiceEngine& VoiceEngine::instance() {
    static VoiceEngine instance;
    return instance;
}

void VoiceEngine::init() {
    // Initialize sub-components
    // whisper = std::make_unique<WhisperStream>();
    // translator = std::make_unique<Translator>();
    // codec = std::make_unique<AudioCodec>();
    // transport = std::make_unique<P2PTransport>();
    std::cout << "[Kipepeo] VoiceEngine initialized" << std::endl;
}

void VoiceEngine::startCall(const std::string& peerId) {
    currentState = CallState::CONNECTING;
    // Logic to connect via P2PTransport
    std::cout << "[Kipepeo] Starting call with " << peerId << std::endl;
    currentState = CallState::CONNECTED; // Mock
}

void VoiceEngine::endCall() {
    currentState = CallState::DISCONNECTED;
    std::cout << "[Kipepeo] Call ended" << std::endl;
    currentState = CallState::IDLE;
}

void VoiceEngine::processAudioFrame(const std::vector<int16_t>& pcmData) {
    if (currentState != CallState::CONNECTED) return;

    // 1. Encode Audio (Opus)
    // auto encoded = codec->encode(pcmData);
    
    // 2. Send to Peer
    // transport->send(encoded);
    
    // 3. Whisper Transcription (Async)
    // if (whisper) {
    //     whisper->pushAudio(pcmData);
    // }
}

void VoiceEngine::setTranslationEnabled(bool enabled) {
    translationEnabled = enabled;
}

void VoiceEngine::setTargetLanguage(const std::string& lang) {
    targetLanguage = lang;
}

void VoiceEngine::setIncomingAudioCallback(AudioCallback callback) {
    // Store callback
}

void VoiceEngine::setTranscribedTextCallback(TextCallback callback) {
    // Store callback
}

void VoiceEngine::setTranslatedTextCallback(TextCallback callback) {
    // Store callback
}

} // namespace voice
} // namespace kipepeo
