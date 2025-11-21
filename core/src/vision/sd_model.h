#pragma once

#include <string>
#include <vector>

namespace kipepeo {
namespace vision {

class SdModel {
public:
    SdModel();
    ~SdModel();

    bool init(const std::string& modelPath);
    std::vector<uint8_t> generate(const std::string& prompt, int steps = 4);
};

} // namespace vision
} // namespace kipepeo
