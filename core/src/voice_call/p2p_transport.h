#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace kipepeo {
namespace voice {

class P2PTransport {
public:
    P2PTransport();
    ~P2PTransport();

    bool init();
    void connect(const std::string& peerId);
    void send(const std::vector<uint8_t>& data);
    
    // Callbacks for receiving data
};

} // namespace voice
} // namespace kipepeo
