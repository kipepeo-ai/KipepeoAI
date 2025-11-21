#include "whisper_stream.h"
#include <iostream>

namespace kipepeo {
namespace voice {

WhisperStream::WhisperStream() {
}

WhisperStream::~WhisperStream() {
}

bool WhisperStream::init(const std::string& modelPath) {
    std::cout << "[Kipepeo] WhisperStream init with " << modelPath << std::endl;
    return true;
}

void WhisperStream::pushAudio(const std::vector<int16_t>& pcmData) {
    // Buffer audio and run inference
}

void WhisperStream::setCallback(TranscriptionCallback callback) {
    // Store callback
}

void WhisperStream::workerThread() {
    // Continuous inference loop
}

} // namespace voice
} // namespace kipepeo
