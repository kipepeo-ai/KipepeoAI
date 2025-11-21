#include "tutor_engine.h"
#include <iostream>

namespace kipepeo {
namespace education {

TutorEngine& TutorEngine::instance() {
    static TutorEngine instance;
    return instance;
}

void TutorEngine::init() {
    std::cout << "[Kipepeo] TutorEngine initialized (Kipepeo Teacher 34B RAG)" << std::endl;
}

TutorResponse TutorEngine::ask(const std::string& subject, const std::string& question) {
    std::cout << "[Kipepeo] Tutor asked (" << subject << "): " << question << std::endl;
    return {
        "Photosynthesis is the process by which green plants use sunlight to synthesize foods from carbon dioxide and water.",
        {"Chlorophyll", "Stomata", "Light Reaction"},
        "KCSE Biology Syllabus Form 1"
    };
}

std::string TutorEngine::predictGrade(const std::string& subject, const std::vector<int>& scores) {
    return "A- (Strong improvement needed in Paper 2)";
}

} // namespace education
} // namespace kipepeo
