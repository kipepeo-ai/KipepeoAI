#include "audio_codec.h"
#include <iostream>

namespace kipepeo {
namespace voice {

AudioCodec::AudioCodec() {
}

AudioCodec::~AudioCodec() {
}

bool AudioCodec::init() {
    std::cout << "[Kipepeo] AudioCodec init (Opus/AV1)" << std::endl;
    return true;
}

std::vector<uint8_t> AudioCodec::encode(const std::vector<int16_t>& pcmData) {
    // Mock encoding
    return std::vector<uint8_t>(pcmData.size() / 2); 
}

std::vector<int16_t> AudioCodec::decode(const std::vector<uint8_t>& encodedData) {
    // Mock decoding
    return std::vector<int16_t>(encodedData.size() * 2);
}

} // namespace voice
} // namespace kipepeo
