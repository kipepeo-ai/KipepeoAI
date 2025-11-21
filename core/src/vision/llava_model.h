#pragma once

#include <string>
#include <vector>

namespace kipepeo {
namespace vision {

class LlavaModel {
public:
    LlavaModel();
    ~LlavaModel();

    bool init(const std::string& modelPath);
    std::string describe(const std::vector<uint8_t>& imageData, const std::string& prompt = "Describe this image");
};

} // namespace vision
} // namespace kipepeo
