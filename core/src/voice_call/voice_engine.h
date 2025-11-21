#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace kipepeo {
namespace voice {

enum class CallState {
    IDLE,
    CONNECTING,
    CONNECTED,
    DISCONNECTED
};

class VoiceEngine {
public:
    static VoiceEngine& instance();

    void init();
    void startCall(const std::string& peerId);
    void endCall();
    
    // Audio processing
    void processAudioFrame(const std::vector<int16_t>& pcmData);
    
    // Translation
    void setTranslationEnabled(bool enabled);
    void setTargetLanguage(const std::string& lang); // "sw", "en", "sheng", "ki", "luo"

    // Callbacks
    using AudioCallback = std::function<void(const std::vector<int16_t>&)>;
    using TextCallback = std::function<void(const std::string&)>;
    
    void setIncomingAudioCallback(AudioCallback callback);
    void setTranscribedTextCallback(TextCallback callback);
    void setTranslatedTextCallback(TextCallback callback);

private:
    VoiceEngine() = default;
    ~VoiceEngine() = default;
    
    CallState currentState = CallState::IDLE;
    bool translationEnabled = false;
    std::string targetLanguage = "sw";
    
    // Sub-components (PIMPL or direct ptrs)
    // std::unique_ptr<WhisperStream> whisper;
    // std::unique_ptr<Translator> translator;
    // std::unique_ptr<AudioCodec> codec;
    // std::unique_ptr<P2PTransport> transport;
};

} // namespace voice
} // namespace kipepeo
