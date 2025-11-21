#include "llava_model.h"
#include <iostream>

namespace kipepeo {
namespace vision {

LlavaModel::LlavaModel() {
}

LlavaModel::~LlavaModel() {
}

bool LlavaModel::init(const std::string& modelPath) {
    std::cout << "[Kipepeo] LLaVA init" << std::endl;
    return true;
}

std::string LlavaModel::describe(const std::vector<uint8_t>& imageData, const std::string& prompt) {
    return "LLaVA description placeholder";
}

} // namespace vision
} // namespace kipepeo
