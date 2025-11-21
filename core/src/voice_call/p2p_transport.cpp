#include "p2p_transport.h"
#include <iostream>

namespace kipepeo {
namespace voice {

P2PTransport::P2PTransport() {
}

P2PTransport::~P2PTransport() {
}

bool P2PTransport::init() {
    std::cout << "[Kipepeo] P2PTransport init (Wi-Fi Direct/BT)" << std::endl;
    return true;
}

void P2PTransport::connect(const std::string& peerId) {
    std::cout << "[Kipepeo] P2P Connecting to " << peerId << std::endl;
}

void P2PTransport::send(const std::vector<uint8_t>& data) {
    // Mock send
}

} // namespace voice
} // namespace kipepeo
