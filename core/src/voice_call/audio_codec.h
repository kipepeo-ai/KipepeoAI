#pragma once

#include <vector>
#include <cstdint>

namespace kipepeo {
namespace voice {

class AudioCodec {
public:
    AudioCodec();
    ~AudioCodec();

    bool init();
    std::vector<uint8_t> encode(const std::vector<int16_t>& pcmData);
    std::vector<int16_t> decode(const std::vector<uint8_t>& encodedData);
};

} // namespace voice
} // namespace kipepeo
