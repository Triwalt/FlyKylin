/**
 * @file ProtobufSerializer.h
 * @brief Protobuf消息序列化器实现（简洁实用版）
 * 
 * 遵循原则：
 * - 只实现当前需要的功能（节点发现、文本消息）
 * - 避免过度抽象
 * - 保持代码简单易懂
 */

#ifndef PROTOBUFSERIALIZER_H
#define PROTOBUFSERIALIZER_H

#include "core/ports/I_MessageSerializer.h"
#include <memory>

namespace flykylin {
namespace protocol {
class PeerInfo;
}
}

namespace flykylin {
namespace adapters {

/**
 * @class ProtobufSerializer
 * @brief Protobuf消息序列化器
 * 
 * 职责：
 * 1. 序列化/反序列化节点发现消息
 * 2. 序列化/反序列化文本消息
 * 3. 处理协议版本兼容性
 */
class ProtobufSerializer : public ports::I_MessageSerializer {
public:
    ProtobufSerializer();
    ~ProtobufSerializer() override;

    // ========== 节点发现消息 ==========
    
    std::vector<uint8_t> serializePeerAnnounce(const core::PeerNode& peer) override;
    std::vector<uint8_t> serializePeerHeartbeat(const core::PeerNode& peer) override;
    std::vector<uint8_t> serializePeerGoodbye(const core::PeerNode& peer) override;
    std::optional<core::PeerNode> deserializePeerMessage(const std::vector<uint8_t>& data) override;

    // ========== 文本消息 ==========
    
    std::vector<uint8_t> serializeTextMessage(const core::Message& message) override;
    std::optional<core::Message> deserializeTextMessage(const std::vector<uint8_t>& data) override;

    // ========== 通用操作 ==========
    
    std::string getFormatName() const override { return "protobuf"; }
    std::string getProtocolVersion() const override { return "1.0.0"; }
    bool isValidMessage(const std::vector<uint8_t>& data) const override;

    bool isValidDiscoveryMessage(const std::vector<uint8_t>& data) const;

private:
    // 辅助方法：将PeerNode转换为protobuf PeerInfo
    void convertToProtobuf(const core::PeerNode& peer, protocol::PeerInfo* pbPeerInfo) const;
    
    // 辅助方法：将protobuf PeerInfo转换为PeerNode
    core::PeerNode convertFromProtobuf(const protocol::PeerInfo& pbPeerInfo) const;
};

} // namespace adapters
} // namespace flykylin

#endif // PROTOBUFSERIALIZER_H
