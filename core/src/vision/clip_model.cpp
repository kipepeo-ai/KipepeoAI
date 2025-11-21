#include "clip_model.h"
#include <iostream>

namespace kipepeo {
namespace vision {

ClipModel::ClipModel() {
}

ClipModel::~ClipModel() {
}

bool ClipModel::init(const std::string& modelPath) {
    std::cout << "[Kipepeo] CLIP init" << std::endl;
    return true;
}

std::vector<float> ClipModel::encodeImage(const std::vector<uint8_t>& imageData) {
    return std::vector<float>(512, 0.1f);
}

std::vector<float> ClipModel::encodeText(const std::string& text) {
    return std::vector<float>(512, 0.1f);
}

} // namespace vision
} // namespace kipepeo
