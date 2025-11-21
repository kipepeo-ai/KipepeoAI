#pragma once

#include <string>
#include <vector>

namespace kipepeo {
namespace vision {

class ClipModel {
public:
    ClipModel();
    ~ClipModel();

    bool init(const std::string& modelPath);
    std::vector<float> encodeImage(const std::vector<uint8_t>& imageData);
    std::vector<float> encodeText(const std::string& text);
};

} // namespace vision
} // namespace kipepeo
