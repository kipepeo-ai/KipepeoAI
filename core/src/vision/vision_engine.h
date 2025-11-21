#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace kipepeo {
namespace vision {

enum class VisionTask {
    IDLE,
    CLASSIFY, // CLIP
    GENERATE, // Stable Diffusion
    DESCRIBE  // LLaVA
};

class VisionEngine {
public:
    static VisionEngine& instance();

    void init();
    
    // Farmer Mode / Textbook Mode
    std::string describeImage(const std::vector<uint8_t>& imageData);
    
    // Image Generation
    std::vector<uint8_t> generateImage(const std::string& prompt);
    
    // Callbacks
    using ProgressCallback = std::function<void(float, const std::string&)>;
    void setProgressCallback(ProgressCallback callback);

private:
    VisionEngine() = default;
    ~VisionEngine() = default;
    
    VisionTask currentTask = VisionTask::IDLE;
    
    // Sub-components
    // std::unique_ptr<ClipModel> clip;
    // std::unique_ptr<SdModel> sd;
    // std::unique_ptr<LlavaModel> llava;
};

} // namespace vision
} // namespace kipepeo
