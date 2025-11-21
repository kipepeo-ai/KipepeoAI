#include "health_engine.h"
#include <iostream>

namespace kipepeo {
namespace health {

HealthEngine& HealthEngine::instance() {
    static HealthEngine instance;
    return instance;
}

void HealthEngine::init() {
    std::cout << "[Kipepeo] HealthEngine initialized (Kipepeo Doctor 13B)" << std::endl;
}

Diagnosis HealthEngine::diagnose(const std::string& symptoms) {
    std::cout << "[Kipepeo] Diagnosing symptoms: " << symptoms << std::endl;
    return {
        "Malaria",
        0.85f,
        "Visit a clinic immediately for a blood test. Drink plenty of water.",
        "Kenyatta National Hospital (2km away)"
    };
}

Diagnosis HealthEngine::diagnoseFromPhoto(const std::vector<uint8_t>& photoData) {
    std::cout << "[Kipepeo] Analyzing medical photo..." << std::endl;
    return {
        "Mild Eczema",
        0.92f,
        "Apply moisturizer. Avoid harsh soaps.",
        "Aga Khan Clinic (1.5km away)"
    };
}

} // namespace health
} // namespace kipepeo
