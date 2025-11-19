/**
 * @file ProtobufSerializer.cpp
 * @brief Protobuf消息序列化器实现
 */

#include "ProtobufSerializer.h"
#include "core/PeerNode.h"
#include "core/Message.h"
#include "messages.pb.h"  // Protobuf生成的头文件

#include <QString>
#include <QDateTime>

namespace flykylin {
namespace adapters {

ProtobufSerializer::ProtobufSerializer() {
    // 初始化Protobuf库（如需要）
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

ProtobufSerializer::~ProtobufSerializer() {
    // 清理Protobuf库（如需要）
    google::protobuf::ShutdownProtobufLibrary();
}

// ========== 节点发现消息 ==========

std::vector<uint8_t> ProtobufSerializer::serializePeerAnnounce(const core::PeerNode& peer) {
    flykylin::protocol::DiscoveryMessage msg;
    msg.set_type(flykylin::protocol::DiscoveryType::ANNOUNCE);
    
    auto* peerInfo = msg.mutable_peer();
    convertToProtobuf(peer, peerInfo);
    
    // 序列化为字节数组
    std::vector<uint8_t> buffer(msg.ByteSizeLong());
    msg.SerializeToArray(buffer.data(), buffer.size());
    return buffer;
}

std::vector<uint8_t> ProtobufSerializer::serializePeerHeartbeat(const core::PeerNode& peer) {
    flykylin::protocol::DiscoveryMessage msg;
    msg.set_type(flykylin::protocol::DiscoveryType::HEARTBEAT);
    
    auto* peerInfo = msg.mutable_peer();
    convertToProtobuf(peer, peerInfo);
    
    std::vector<uint8_t> buffer(msg.ByteSizeLong());
    msg.SerializeToArray(buffer.data(), buffer.size());
    return buffer;
}

std::vector<uint8_t> ProtobufSerializer::serializePeerGoodbye(const core::PeerNode& peer) {
    flykylin::protocol::DiscoveryMessage msg;
    msg.set_type(flykylin::protocol::DiscoveryType::GOODBYE);
    
    auto* peerInfo = msg.mutable_peer();
    convertToProtobuf(peer, peerInfo);
    
    std::vector<uint8_t> buffer(msg.ByteSizeLong());
    msg.SerializeToArray(buffer.data(), buffer.size());
    return buffer;
}

std::optional<core::PeerNode> ProtobufSerializer::deserializePeerMessage(const std::vector<uint8_t>& data) {
    flykylin::protocol::DiscoveryMessage msg;
    
    // 反序列化
    if (!msg.ParseFromArray(data.data(), data.size())) {
        return std::nullopt;  // 解析失败
    }
    
    // 检查消息类型
    if (!msg.has_peer()) {
        return std::nullopt;
    }
    
    // 转换为PeerNode
    return convertFromProtobuf(msg.peer());
}

// ========== 文本消息 ==========

std::vector<uint8_t> ProtobufSerializer::serializeTextMessage(const core::Message& message) {
    flykylin::protocol::TcpMessage wrapper;
    wrapper.set_type(flykylin::protocol::TcpMessage::TEXT);
    
    flykylin::protocol::TextMessage textMsg;
    textMsg.set_message_id(message.id().toStdString());
    textMsg.set_from_user_id(message.fromUserId().toStdString());
    textMsg.set_to_user_id(message.toUserId().toStdString());
    textMsg.set_content(message.content().toStdString());
    textMsg.set_timestamp(message.timestamp().toMSecsSinceEpoch());
    textMsg.set_is_group(false);  // 简化：只支持1v1
    
    // 序列化TextMessage到wrapper的payload
    std::string payload;
    textMsg.SerializeToString(&payload);
    wrapper.set_payload(payload);
    
    // 序列化wrapper
    std::vector<uint8_t> buffer(wrapper.ByteSizeLong());
    wrapper.SerializeToArray(buffer.data(), buffer.size());
    return buffer;
}

std::optional<core::Message> ProtobufSerializer::deserializeTextMessage(const std::vector<uint8_t>& data) {
    flykylin::protocol::TcpMessage wrapper;
    
    // 反序列化wrapper
    if (!wrapper.ParseFromArray(data.data(), data.size())) {
        return std::nullopt;
    }
    
    // 检查消息类型
    if (wrapper.type() != flykylin::protocol::TcpMessage::TEXT) {
        return std::nullopt;
    }
    
    // 反序列化payload
    flykylin::protocol::TextMessage textMsg;
    if (!textMsg.ParseFromString(wrapper.payload())) {
        return std::nullopt;
    }
    
    // 转换为Message对象
    core::Message message;
    message.setId(QString::fromStdString(textMsg.message_id()));
    message.setFromUserId(QString::fromStdString(textMsg.from_user_id()));
    message.setToUserId(QString::fromStdString(textMsg.to_user_id()));
    message.setContent(QString::fromStdString(textMsg.content()));
    message.setTimestamp(QDateTime::fromMSecsSinceEpoch(textMsg.timestamp()));
    
    return message;
}

// ========== 通用操作 ==========

bool ProtobufSerializer::isValidMessage(const std::vector<uint8_t>& data) const {
    if (isValidDiscoveryMessage(data)) {
        return true;
    }

    flykylin::protocol::TcpMessage wrapper;
    return wrapper.ParseFromArray(data.data(), data.size());
}

bool ProtobufSerializer::isValidDiscoveryMessage(const std::vector<uint8_t>& data) const {
    flykylin::protocol::DiscoveryMessage msg;
    if (!msg.ParseFromArray(data.data(), data.size())) {
        return false;
    }
    return msg.has_peer();
}

// ========== 私有辅助方法 ==========

void ProtobufSerializer::convertToProtobuf(const core::PeerNode& peer, flykylin::protocol::PeerInfo* peerInfo) const {
    peerInfo->set_user_id(peer.userId().toStdString());
    peerInfo->set_user_name(peer.userName().toStdString());
    peerInfo->set_ip_address(peer.ipAddress().toString().toStdString());
    peerInfo->set_port(peer.port());
    peerInfo->set_timestamp(peer.lastSeenTime().toMSecsSinceEpoch());
    peerInfo->set_os_type(peer.osType().toStdString());
    peerInfo->set_version("1.0.0");  // 简化：固定版本号
}

core::PeerNode ProtobufSerializer::convertFromProtobuf(const flykylin::protocol::PeerInfo& peerInfo) const {
    core::PeerNode peer;
    peer.setUserId(QString::fromStdString(peerInfo.user_id()));
    peer.setUserName(QString::fromStdString(peerInfo.user_name()));
    peer.setIpAddress(QString::fromStdString(peerInfo.ip_address()));
    peer.setPort(peerInfo.port());
    peer.setLastSeenTime(QDateTime::fromMSecsSinceEpoch(peerInfo.timestamp()));
    peer.setOsType(QString::fromStdString(peerInfo.os_type()));
    
    return peer;
}

} // namespace adapters
} // namespace flykylin
