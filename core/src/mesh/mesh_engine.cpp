#include "mesh_engine.h"
#include <iostream>

namespace kipepeo {
namespace mesh {

MeshEngine& MeshEngine::instance() {
    static MeshEngine instance;
    return instance;
}

void MeshEngine::init() {
    std::cout << "[Kipepeo] MeshEngine initialized (Serval DNA Mock)" << std::endl;
}

void MeshEngine::startDiscovery() {
    std::cout << "[Kipepeo] Starting mesh peer discovery (Wi-Fi Direct + BT)..." << std::endl;
}

void MeshEngine::stopDiscovery() {
    std::cout << "[Kipepeo] Stopping mesh peer discovery." << std::endl;
}

std::vector<Peer> MeshEngine::getPeers() {
    // Mock peers
    return {
        {"peer1", "Juma (Tecno Spark)", 85, true},
        {"peer2", "Amani (Samsung A14)", 60, true},
        {"peer3", "Wanjiku (Infinix)", 30, false}
    };
}

void MeshEngine::sendMessage(const std::string& peerId, const std::string& message) {
    std::cout << "[Kipepeo] Sending mesh message to " << peerId << ": " << message << std::endl;
}

} // namespace mesh
} // namespace kipepeo
