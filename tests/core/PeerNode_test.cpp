/**
 * @file PeerNode_test.cpp
 * @brief PeerNode类的单元测试
 */

#include "core/models/PeerNode.h"
#include <gtest/gtest.h>
#include <QHostAddress>
#include <QThread>

using namespace flykylin::core;

/**
 * @brief PeerNode基础功能测试
 */
class PeerNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试节点
        testNode = PeerNode(
            "192.168.1.100",
            "TestUser",
            "TestHost",
            QHostAddress("192.168.1.100"),
            45679
        );
    }

    PeerNode testNode;
};

// 测试构造函数
TEST_F(PeerNodeTest, Constructor_ValidParameters_CreatesNode) {
    EXPECT_EQ(testNode.userId(), "192.168.1.100");
    EXPECT_EQ(testNode.userName(), "TestUser");
    EXPECT_EQ(testNode.hostName(), "TestHost");
    EXPECT_EQ(testNode.ipAddress(), QHostAddress("192.168.1.100"));
    EXPECT_EQ(testNode.tcpPort(), 45679);
    EXPECT_TRUE(testNode.isOnline());
    EXPECT_TRUE(testNode.lastSeen().isValid());
}

// 测试默认构造函数
TEST_F(PeerNodeTest, DefaultConstructor_CreatesEmptyNode) {
    PeerNode node;
    EXPECT_TRUE(node.userId().isEmpty());
    EXPECT_TRUE(node.userName().isEmpty());
    EXPECT_EQ(node.tcpPort(), 0);
    EXPECT_FALSE(node.lastSeen().isValid());  // 默认构造的lastSeen应该无效
    EXPECT_TRUE(node.isOnline());  // 默认在线状态为true（C++11成员初始化）
}

// 测试Setter方法
TEST_F(PeerNodeTest, Setters_UpdateValues_Successfully) {
    testNode.setUserName("UpdatedUser");
    EXPECT_EQ(testNode.userName(), "UpdatedUser");
    
    testNode.setOnline(false);
    EXPECT_FALSE(testNode.isOnline());
}

// 测试updateLastSeen
TEST_F(PeerNodeTest, UpdateLastSeen_UpdatesTimestamp_AndSetsOnline) {
    QDateTime oldTime = testNode.lastSeen();
    
    // 等待1毫秒确保时间变化
    QThread::msleep(2);
    
    testNode.updateLastSeen();
    
    EXPECT_GT(testNode.lastSeen(), oldTime);
    EXPECT_TRUE(testNode.isOnline());
}

// 测试超时检测 - 未超时
TEST_F(PeerNodeTest, IsTimedOut_WithinTimeout_ReturnsFalse) {
    testNode.updateLastSeen();
    EXPECT_FALSE(testNode.isTimedOut(30));
}

// 测试超时检测 - 已超时
TEST_F(PeerNodeTest, IsTimedOut_BeyondTimeout_ReturnsTrue) {
    // 设置过去的时间
    QDateTime pastTime = QDateTime::currentDateTime().addSecs(-35);
    testNode.setLastSeen(pastTime);
    
    EXPECT_TRUE(testNode.isTimedOut(30));
}

// 测试超时检测 - 无效时间
TEST_F(PeerNodeTest, IsTimedOut_InvalidTime_ReturnsTrue) {
    PeerNode node;
    node.setLastSeen(QDateTime());  // 无效时间
    
    EXPECT_TRUE(node.isTimedOut(30));
}

// 测试相等运算符
TEST_F(PeerNodeTest, EqualityOperator_SameUserId_ReturnsTrue) {
    PeerNode otherNode(
        "192.168.1.100",  // 相同的userId
        "DifferentUser",
        "DifferentHost",
        QHostAddress("192.168.1.101"),
        12345
    );
    
    EXPECT_TRUE(testNode == otherNode);
}

// 测试不等运算符
TEST_F(PeerNodeTest, InequalityOperator_DifferentUserId_ReturnsTrue) {
    PeerNode otherNode(
        "192.168.1.101",  // 不同的userId
        "TestUser",
        "TestHost",
        QHostAddress("192.168.1.101"),
        45679
    );
    
    EXPECT_TRUE(testNode != otherNode);
}

// 性能测试 - 创建大量节点
TEST_F(PeerNodeTest, Performance_Create1000Nodes_CompletesQuickly) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        QString userId = QString("192.168.1.%1").arg(i);
        PeerNode node(userId, "User", "Host", QHostAddress(userId), 45679);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 应该在500ms内完成（放宽阈值以适应不同性能的机器）
    EXPECT_LT(duration.count(), 500);
    
    // 打印性能信息用于调试
    std::cout << "Created 1000 nodes in " << duration.count() << "ms" << std::endl;
}
