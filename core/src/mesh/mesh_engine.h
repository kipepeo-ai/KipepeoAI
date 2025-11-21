#pragma once

#include <string>
#include <vector>

namespace kipepeo {
namespace mesh {

struct Peer {
    std::string id;
    std::string name;
    int signalStrength; // 0-100
    bool isConnected;
};

class MeshEngine {
public:
    static MeshEngine& instance();

    void init();
    void startDiscovery();
    void stopDiscovery();
    
    std::vector<Peer> getPeers();
    void sendMessage(const std::string& peerId, const std::string& message);

private:
    MeshEngine() = default;
    ~MeshEngine() = default;
};

} // namespace mesh
} // namespace kipepeo
