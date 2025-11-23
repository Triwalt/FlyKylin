/**
 * @file I_MessageSerializer.h
 * @brief 消息序列化器接口（六边形架构 - Port）
 * 
 * 定义消息序列化/反序列化的抽象接口，与具体实现（Protobuf/JSON）解耦
 */

#ifndef I_MESSAGESERIALIZER_H
#define I_MESSAGESERIALIZER_H

#include <cstdint>
#include <vector>
#include <memory>
#include <optional>
#include <string>

#include "core/models/PeerNode.h"
#include "core/models/Message.h"

namespace flykylin {

// 前向声明核心数据模型
namespace core {
    class PeerNode;
    class Message;
}

namespace ports {

/**
 * @class I_MessageSerializer
 * @brief 消息序列化器接口
 * 
 * 定义消息序列化和反序列化操作，支持：
 * - 节点发现消息
 * - 文本消息
 * - 文件传输消息
 * 
 * 实现类：
 * - ProtobufSerializer（使用Protocol Buffers）
 * - JsonSerializer（使用JSON，用于调试）
 */
class I_MessageSerializer {
public:
    virtual ~I_MessageSerializer() = default;

    // ========== 节点发现消息 ==========
    
    /**
     * @brief 序列化节点公告消息（UDP广播）
     * @param peer 节点信息
     * @return 序列化后的字节数组
     */
    virtual std::vector<uint8_t> serializePeerAnnounce(const core::PeerNode& peer) = 0;
    
    /**
     * @brief 序列化节点心跳消息（UDP广播）
     * @param peer 节点信息
     * @return 序列化后的字节数组
     */
    virtual std::vector<uint8_t> serializePeerHeartbeat(const core::PeerNode& peer) = 0;
    
    /**
     * @brief 序列化节点下线消息（UDP广播）
     * @param peer 节点信息
     * @return 序列化后的字节数组
     */
    virtual std::vector<uint8_t> serializePeerGoodbye(const core::PeerNode& peer) = 0;
    
    /**
     * @brief 反序列化节点发现消息
     * @param data 字节数组
     * @return 节点信息，失败返回nullopt
     */
    virtual std::optional<core::PeerNode> deserializePeerMessage(const std::vector<uint8_t>& data) = 0;

    // ========== 文本消息 ==========
    
    /**
     * @brief 序列化文本消息（TCP）
     * @param message 消息对象
     * @return 序列化后的字节数组
     */
    virtual std::vector<uint8_t> serializeTextMessage(const core::Message& message) = 0;
    
    /**
     * @brief 反序列化文本消息
     * @param data 字节数组
     * @return 消息对象，失败返回nullopt
     */
    virtual std::optional<core::Message> deserializeTextMessage(const std::vector<uint8_t>& data) = 0;

    // ========== 通用操作 ==========
    
    /**
     * @brief 获取序列化格式名称
     * @return 格式名称（如 "protobuf", "json"）
     */
    virtual std::string getFormatName() const = 0;
    
    /**
     * @brief 获取协议版本号
     * @return 版本号（如 "1.0.0"）
     */
    virtual std::string getProtocolVersion() const = 0;
    
    /**
     * @brief 验证消息格式是否有效
     * @param data 字节数组
     * @return 有效返回true
     */
    virtual bool isValidMessage(const std::vector<uint8_t>& data) const = 0;
};

} // namespace ports
} // namespace flykylin

#endif // I_MESSAGESERIALIZER_H
