#pragma once

#include <cstdint>
#include <vector>

// Forward declaration for opaque pointer
struct VideoToolboxContext;

class VideoToolboxDecoder {
public:
    VideoToolboxDecoder();
    ~VideoToolboxDecoder();

    bool init(int width, int height, const std::vector<uint8_t>& extraData);
    bool decode(const uint8_t* data, size_t size, int64_t pts);
    void flush();

private:
    VideoToolboxContext* ctx;
};
