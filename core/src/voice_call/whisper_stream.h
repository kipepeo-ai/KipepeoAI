#pragma once

#include <vector>
#include <string>
#include <functional>

namespace kipepeo {
namespace voice {

class WhisperStream {
public:
    WhisperStream();
    ~WhisperStream();

    bool init(const std::string& modelPath);
    void pushAudio(const std::vector<int16_t>& pcmData);
    
    using TranscriptionCallback = std::function<void(const std::string&)>;
    void setCallback(TranscriptionCallback callback);

private:
    void workerThread();
    // whisper_context* ctx;
};

} // namespace voice
} // namespace kipepeo
