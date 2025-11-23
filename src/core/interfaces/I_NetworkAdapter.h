#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

// Forward declaration
namespace flykylin::core::models {
    class Message;
    class PeerNode;
}

namespace flykylin::core::interfaces {

/**
 * @brief Network Adapter Interface
 * 
 * Defines the contract for network operations (UDP Discovery, TCP Messaging).
 * Implementations will wrap platform-specific or library-specific (Qt/Boost) sockets.
 */
class I_NetworkAdapter {
public:
    virtual ~I_NetworkAdapter() = default;

    // UDP Discovery
    virtual void startDiscovery(uint16_t port) = 0;
    virtual void stopDiscovery() = 0;
    virtual void sendBroadcast(const std::vector<uint8_t>& data) = 0;
    
    // TCP Messaging
    virtual void startListening(uint16_t port) = 0;
    virtual void connectToPeer(const std::string& ip, uint16_t port) = 0;
    virtual void sendMessage(const std::string& peerId, const std::vector<uint8_t>& data) = 0;

    // Callbacks/Signals (using std::function for pure C++ decoupling)
    using DataReceivedCallback = std::function<void(const std::vector<uint8_t>& data, const std::string& senderIp, uint16_t senderPort)>;
    using PeerConnectedCallback = std::function<void(const std::string& peerId)>;
    using PeerDisconnectedCallback = std::function<void(const std::string& peerId)>;

    virtual void setOnDiscoveryDataReceived(DataReceivedCallback callback) = 0;
    virtual void setOnMessageReceived(DataReceivedCallback callback) = 0;
    virtual void setOnPeerConnected(PeerConnectedCallback callback) = 0;
    virtual void setOnPeerDisconnected(PeerDisconnectedCallback callback) = 0;
};

} // namespace flykylin::core::interfaces
