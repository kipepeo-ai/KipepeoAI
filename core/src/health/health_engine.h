#pragma once

#include <string>
#include <vector>

namespace kipepeo {
namespace health {

struct Diagnosis {
    std::string condition;
    float confidence;
    std::string recommendation;
    std::string nearestClinic;
};

class HealthEngine {
public:
    static HealthEngine& instance();

    void init();
    
    // Symptom Checker
    Diagnosis diagnose(const std::string& symptoms);
    
    // Photo Diagnosis (e.g. skin condition)
    Diagnosis diagnoseFromPhoto(const std::vector<uint8_t>& photoData);

private:
    HealthEngine() = default;
    ~HealthEngine() = default;
};

} // namespace health
} // namespace kipepeo
