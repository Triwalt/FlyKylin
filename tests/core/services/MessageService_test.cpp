/**
 * @file MessageService_test.cpp
 * @brief MessageService单元测试
 * @author FlyKylin Development Team
 * @date 2024-11-20
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include "core/services/MessageService.h"
#include "core/config/UserProfile.h"
#include "core/models/Message.h"

using namespace flykylin;
using namespace flykylin::services;
using namespace flykylin::core;

/**
 * @brief MessageService测试夹具
 */
class MessageServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化Qt应用上下文（如果还没有）
        if (!QCoreApplication::instance()) {
            static int argc = 0;
            static char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }
        
        // 初始化UserProfile（userId由系统自动生成）
        UserProfile& profile = UserProfile::instance();
        profile.setUserName("TestUser");
        
        // 创建MessageService实例
        messageService = std::make_unique<MessageService>();
        
        // 设置测试对等节点
        testPeerId = "peer-456";
        testContent = "Hello, FlyKylin Test!";
    }
    
    void TearDown() override {
        messageService.reset();
    }
    
    std::unique_ptr<MessageService> messageService;
    static QCoreApplication* app;
    QString testPeerId;
    QString testContent;
};

QCoreApplication* MessageServiceTest::app = nullptr;

// ========== 基础功能测试 ==========

/**
 * @brief 测试MessageService初始化
 */
TEST_F(MessageServiceTest, Initialization_Success) {
    ASSERT_NE(messageService, nullptr);
}

/**
 * @brief 测试发送空消息（应拒绝）
 */
TEST_F(MessageServiceTest, SendEmptyMessage_ShouldBeRejected) {
    // 尝试发送空消息
    messageService->sendTextMessage(testPeerId, "");
    
    // 验证：消息历史应为空
    auto history = messageService->getMessageHistory(testPeerId);
    EXPECT_TRUE(history.isEmpty());
}

/**
 * @brief 测试发送纯空白消息（应拒绝）
 */
TEST_F(MessageServiceTest, SendWhitespaceMessage_ShouldBeRejected) {
    messageService->sendTextMessage(testPeerId, "   \t\n  ");
    
    auto history = messageService->getMessageHistory(testPeerId);
    EXPECT_TRUE(history.isEmpty());
}

/**
 * @brief 测试消息信号发射
 */
TEST_F(MessageServiceTest, SendMessage_EmitsSignal) {
    // 创建信号监听器
    QSignalSpy spyReceived(messageService.get(), &MessageService::messageReceived);
    QSignalSpy spySent(messageService.get(), &MessageService::messageSent);
    QSignalSpy spyFailed(messageService.get(), &MessageService::messageFailed);
    
    // 发送消息到Echo Bot（本地测试）
    QString echoBotId = LocalEchoService::getEchoBotId();
    messageService->sendTextMessage(echoBotId, testContent);
    
    // 等待信号（Echo Bot会立即回复）
    EXPECT_TRUE(spyReceived.wait(1000));
    
    // 验证：至少收到1条消息（Echo Bot的回复）
    EXPECT_GE(spyReceived.count(), 1);
}

// ========== 消息历史测试 ==========

/**
 * @brief 测试消息历史存储
 */
TEST_F(MessageServiceTest, MessageHistory_StoresMessages) {
    // 发送消息到Echo Bot
    QString echoBotId = LocalEchoService::getEchoBotId();
    messageService->sendTextMessage(echoBotId, testContent);
    
    // 等待处理
    QTest::qWait(100);
    
    // 获取消息历史
    auto history = messageService->getMessageHistory(echoBotId);
    
    // 验证：至少有1条消息
    EXPECT_GE(history.size(), 1);
}

/**
 * @brief 测试清除消息历史
 */
TEST_F(MessageServiceTest, ClearHistory_RemovesMessages) {
    // 发送消息
    QString echoBotId = LocalEchoService::getEchoBotId();
    messageService->sendTextMessage(echoBotId, testContent);
    QTest::qWait(100);
    
    // 清除历史
    messageService->clearHistory(echoBotId);
    
    // 验证：历史为空
    auto history = messageService->getMessageHistory(echoBotId);
    EXPECT_TRUE(history.isEmpty());
}

/**
 * @brief 测试多条消息存储顺序
 */
TEST_F(MessageServiceTest, MultipleMessages_PreserveOrder) {
    QString echoBotId = LocalEchoService::getEchoBotId();
    
    // 发送多条消息
    QString msg1 = "Message 1";
    QString msg2 = "Message 2";
    QString msg3 = "Message 3";
    
    messageService->sendTextMessage(echoBotId, msg1);
    QTest::qWait(50);
    messageService->sendTextMessage(echoBotId, msg2);
    QTest::qWait(50);
    messageService->sendTextMessage(echoBotId, msg3);
    QTest::qWait(100);
    
    // 获取历史
    auto history = messageService->getMessageHistory(echoBotId);
    
    // 验证：消息数量正确（发送的3条 + Echo Bot回复的3条 = 至少6条）
    EXPECT_GE(history.size(), 6);
}

// ========== 消息字段验证测试 ==========

/**
 * @brief 测试消息字段完整性
 */
TEST_F(MessageServiceTest, Message_HasCompleteFields) {
    QString echoBotId = LocalEchoService::getEchoBotId();
    
    // 创建信号监听器
    QSignalSpy spyReceived(messageService.get(), &MessageService::messageReceived);
    
    // 发送消息
    messageService->sendTextMessage(echoBotId, testContent);
    
    // 等待接收
    ASSERT_TRUE(spyReceived.wait(1000));
    
    // 获取接收到的消息
    auto arguments = spyReceived.takeFirst();
    auto message = qvariant_cast<Message>(arguments.at(0));
    
    // 验证：所有字段都有效
    EXPECT_FALSE(message.id().isEmpty());
    EXPECT_FALSE(message.fromUserId().isEmpty());
    EXPECT_FALSE(message.toUserId().isEmpty());
    EXPECT_FALSE(message.content().isEmpty());
    EXPECT_TRUE(message.timestamp().isValid());
}

/**
 * @brief 测试消息状态转换
 */
TEST_F(MessageServiceTest, MessageStatus_ValidTransitions) {
    QString echoBotId = LocalEchoService::getEchoBotId();
    
    // 创建信号监听器
    QSignalSpy spyReceived(messageService.get(), &MessageService::messageReceived);
    
    messageService->sendTextMessage(echoBotId, testContent);
    ASSERT_TRUE(spyReceived.wait(1000));
    
    // 获取消息历史
    auto history = messageService->getMessageHistory(echoBotId);
    ASSERT_GE(history.size(), 1);
    
    // 验证：接收到的消息状态应为Delivered
    auto receivedMsg = history.last();
    EXPECT_EQ(receivedMsg.status(), MessageStatus::Delivered);
}

// ========== Echo Bot集成测试 ==========

/**
 * @brief 测试Echo Bot响应
 */
TEST_F(MessageServiceTest, EchoBot_RespondsCorrectly) {
    QString echoBotId = LocalEchoService::getEchoBotId();
    QSignalSpy spyReceived(messageService.get(), &MessageService::messageReceived);
    
    // 发送消息
    messageService->sendTextMessage(echoBotId, testContent);
    
    // 等待Echo Bot回复
    ASSERT_TRUE(spyReceived.wait(1000));
    
    // 获取消息历史
    auto history = messageService->getMessageHistory(echoBotId);
    
    // 验证：至少有2条消息（发送的 + Echo Bot回复）
    EXPECT_GE(history.size(), 2);
    
    // 验证：Echo Bot的回复包含原消息内容
    bool foundEcho = false;
    for (const auto& msg : history) {
        if (msg.fromUserId() == echoBotId && 
            msg.content().contains(testContent)) {
            foundEcho = true;
            break;
        }
    }
    EXPECT_TRUE(foundEcho);
}

/**
 * @brief 测试Echo Bot响应内容包含原消息
 */
TEST_F(MessageServiceTest, EchoBot_PingCommand) {
    QString echoBotId = LocalEchoService::getEchoBotId();
    QSignalSpy spyReceived(messageService.get(), &MessageService::messageReceived);
    
    messageService->sendTextMessage(echoBotId, "/ping");
    ASSERT_TRUE(spyReceived.wait(1000));
    
    auto history = messageService->getMessageHistory(echoBotId);
    ASSERT_GE(history.size(), 1);
    
    // 查找pong回复
    bool foundPong = false;
    for (const auto& msg : history) {
        if (msg.content().contains("pong") || msg.content().contains("Pong")) {
            foundPong = true;
            break;
        }
    }
    EXPECT_TRUE(foundPong);
}

// ========== 性能测试 ==========

/**
 * @brief 测试批量消息处理性能
 */
TEST_F(MessageServiceTest, Performance_BulkMessages) {
    QString echoBotId = LocalEchoService::getEchoBotId();
    
    const int messageCount = 100;
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    
    // 发送大量消息
    for (int i = 0; i < messageCount; ++i) {
        messageService->sendTextMessage(echoBotId, 
            QString("Test message %1").arg(i));
    }
    
    // 等待处理
    QTest::qWait(2000);
    
    auto endTime = QDateTime::currentMSecsSinceEpoch();
    auto duration = endTime - startTime;
    
    // 验证：性能合理（应在3秒内完成100条消息）
    EXPECT_LT(duration, 3000);
    
    // 验证：所有消息都被处理
    auto history = messageService->getMessageHistory(echoBotId);
    EXPECT_GE(history.size(), messageCount);
}

/**
 * @brief 主函数
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
