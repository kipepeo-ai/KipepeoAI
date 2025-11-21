#pragma once

#include <string>
#include <vector>

namespace kipepeo {
namespace education {

struct TutorResponse {
    std::string answer;
    std::vector<std::string> relatedTopics;
    std::string sourceMaterial; // e.g. "KCSE 2015 Biology Q2"
};

class TutorEngine {
public:
    static TutorEngine& instance();

    void init();
    
    TutorResponse ask(const std::string& subject, const std::string& question);
    std::string predictGrade(const std::string& subject, const std::vector<int>& scores);

private:
    TutorEngine() = default;
    ~TutorEngine() = default;
};

} // namespace education
} // namespace kipepeo
