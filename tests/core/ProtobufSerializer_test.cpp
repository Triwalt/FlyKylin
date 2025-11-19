/**
 * @file ProtobufSerializer_test.cpp
 * @brief ProtobufSerializer单元测试（简洁实用版）
 */

#include <gtest/gtest.h>
#include "core/adapters/ProtobufSerializer.h"
#include "core/PeerNode.h"
#include "core/Message.h"

#include <QDateTime>
#include <QString>

using namespace flykylin;
using namespace flykylin::adapters;

/**
 * @brief 测试夹具：提供通用的测试数据
 */
class ProtobufSerializerTest : public ::testing::Test {
protected:
    void SetUp() override {
        serializer = std::make_unique<ProtobufSerializer>();
        
        // 准备测试用PeerNode
        testPeer.setUserId("user-123");
        testPeer.setUserName("TestUser");
        testPeer.setIpAddress("192.168.1.100");
        testPeer.setPort(12345);
        testPeer.setOsType("Windows");
        testPeer.setLastSeenTime(QDateTime::currentDateTime());
        
        // 准备测试用Message
        testMessage.setId("msg-456");
        testMessage.setFromUserId("user-123");
        testMessage.setToUserId("user-789");
        testMessage.setContent("Hello, FlyKylin!");
        testMessage.setTimestamp(QDateTime::currentDateTime());
    }

    std::unique_ptr<ProtobufSerializer> serializer;
    core::PeerNode testPeer;
    core::Message testMessage;
};

// ========== 节点发现消息测试 ==========

TEST_F(ProtobufSerializerTest, SerializePeerAnnounce_Success) {
    // 序列化
    auto data = serializer->serializePeerAnnounce(testPeer);
    
    // 验证：数据不为空
    EXPECT_FALSE(data.empty());
    
    // 验证：数据有效
    EXPECT_TRUE(serializer->isValidDiscoveryMessage(data));
}

TEST_F(ProtobufSerializerTest, DeserializePeerMessage_Success) {
    // 序列化后反序列化
    auto data = serializer->serializePeerAnnounce(testPeer);
    auto result = serializer->deserializePeerMessage(data);
    
    // 验证：反序列化成功
    ASSERT_TRUE(result.has_value());
    
    // 验证：数据一致
    const auto& peer = result.value();
    EXPECT_EQ(peer.userId(), testPeer.userId());
    EXPECT_EQ(peer.userName(), testPeer.userName());
    EXPECT_EQ(peer.ipAddress(), testPeer.ipAddress());
    EXPECT_EQ(peer.port(), testPeer.port());
    EXPECT_EQ(peer.osType(), testPeer.osType());
}

TEST_F(ProtobufSerializerTest, SerializePeerHeartbeat_Success) {
    auto data = serializer->serializePeerHeartbeat(testPeer);
    EXPECT_FALSE(data.empty());
    EXPECT_TRUE(serializer->isValidDiscoveryMessage(data));
}

TEST_F(ProtobufSerializerTest, SerializePeerGoodbye_Success) {
    auto data = serializer->serializePeerGoodbye(testPeer);
    EXPECT_FALSE(data.empty());
    EXPECT_TRUE(serializer->isValidDiscoveryMessage(data));
}

// ========== 文本消息测试 ==========

TEST_F(ProtobufSerializerTest, SerializeTextMessage_Success) {
    // 序列化
    auto data = serializer->serializeTextMessage(testMessage);
    
    // 验证：数据不为空
    EXPECT_FALSE(data.empty());
    
    // 验证：数据有效
    EXPECT_TRUE(serializer->isValidMessage(data));
}

TEST_F(ProtobufSerializerTest, DeserializeTextMessage_Success) {
    // 序列化后反序列化
    auto data = serializer->serializeTextMessage(testMessage);
    auto result = serializer->deserializeTextMessage(data);
    
    // 验证：反序列化成功
    ASSERT_TRUE(result.has_value());
    
    // 验证：数据一致
    const auto& msg = result.value();
    EXPECT_EQ(msg.id(), testMessage.id());
    EXPECT_EQ(msg.fromUserId(), testMessage.fromUserId());
    EXPECT_EQ(msg.toUserId(), testMessage.toUserId());
    EXPECT_EQ(msg.content(), testMessage.content());
}

// ========== 错误处理测试 ==========

TEST_F(ProtobufSerializerTest, DeserializePeerMessage_InvalidData_ReturnsNullopt) {
    // 无效数据
    std::vector<uint8_t> invalidData = {0xFF, 0xFE, 0xFD};
    
    auto result = serializer->deserializePeerMessage(invalidData);
    
    // 验证：返回nullopt
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(serializer->isValidDiscoveryMessage(invalidData));
}

TEST_F(ProtobufSerializerTest, DeserializeTextMessage_InvalidData_ReturnsNullopt) {
    // 无效数据
    std::vector<uint8_t> invalidData = {0x01, 0x02, 0x03};
    
    auto result = serializer->deserializeTextMessage(invalidData);
    
    // 验证：返回nullopt
    EXPECT_FALSE(result.has_value());
}

TEST_F(ProtobufSerializerTest, IsValidMessage_InvalidData_ReturnsFalse) {
    std::vector<uint8_t> invalidData = {0xAA, 0xBB, 0xCC};

    EXPECT_FALSE(serializer->isValidMessage(invalidData));
}

// ========== 通用属性测试 ==========

TEST_F(ProtobufSerializerTest, GetFormatName_ReturnsProtobuf) {
    EXPECT_EQ(serializer->getFormatName(), "protobuf");
}

TEST_F(ProtobufSerializerTest, GetProtocolVersion_ReturnsVersion) {
    EXPECT_EQ(serializer->getProtocolVersion(), "1.0.0");
}

// ========== 性能测试（简单） ==========

TEST_F(ProtobufSerializerTest, SerializationPerformance_1000Messages) {
    // 测试1000次序列化的性能
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        auto data = serializer->serializeTextMessage(testMessage);
        (void)data;  // 避免未使用警告
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证：1000次序列化应在1秒内完成
    EXPECT_LT(duration.count(), 1000) << "Serialization too slow: " << duration.count() << "ms";
}
