#pragma once

#include <string>

namespace kipepeo {
namespace voice {

class Translator {
public:
    Translator();
    ~Translator();

    bool init(const std::string& modelPath);
    std::string translate(const std::string& text, const std::string& sourceLang, const std::string& targetLang);
};

} // namespace voice
} // namespace kipepeo
