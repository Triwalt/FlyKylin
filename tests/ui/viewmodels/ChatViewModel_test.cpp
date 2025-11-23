/**
 * @file ChatViewModel_test.cpp
 * @brief ChatViewModel单元测试
 * @author FlyKylin Development Team
 * @date 2024-11-20
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include "ui/viewmodels/ChatViewModel.h"
#include "core/config/UserProfile.h"
#include "core/services/LocalEchoService.h"

using namespace flykylin;
using namespace flykylin::ui;
using namespace flykylin::core;

/**
 * @brief ChatViewModel测试夹具
 */
class ChatViewModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化Qt应用上下文
        if (!QCoreApplication::instance()) {
            static int argc = 0;
            static char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }
        
        // 初始化UserProfile
        UserProfile& profile = UserProfile::instance();
        ASSERT_FALSE(profile.userId().isEmpty());
        profile.setUserName("TestUser");
        
        // 创建ChatViewModel
        viewModel = std::make_unique<ChatViewModel>();
        
        // 测试对等节点
        testPeerId = "peer-456";
        testPeerName = "TestPeer";
    }
    
    void TearDown() override {
        viewModel.reset();
    }
    
    std::unique_ptr<ChatViewModel> viewModel;
    static QCoreApplication* app;
    QString testPeerId;
    QString testPeerName;
};

QCoreApplication* ChatViewModelTest::app = nullptr;

// ========== 初始化测试 ==========

TEST_F(ChatViewModelTest, Initialization_Success) {
    ASSERT_NE(viewModel, nullptr);
    EXPECT_TRUE(viewModel->getCurrentPeerId().isEmpty());
    EXPECT_TRUE(viewModel->getCurrentPeerName().isEmpty());
}

TEST_F(ChatViewModelTest, InitialMessageList_IsEmpty) {
    auto messages = viewModel->getMessages();
    EXPECT_TRUE(messages.isEmpty());
}

// ========== 对等节点选择测试 ==========

TEST_F(ChatViewModelTest, SelectPeer_UpdatesCurrentPeer) {
    viewModel->setCurrentPeer(testPeerId, testPeerName);
    
    EXPECT_EQ(viewModel->getCurrentPeerId(), testPeerId);
    EXPECT_EQ(viewModel->getCurrentPeerName(), testPeerName);
}

TEST_F(ChatViewModelTest, SelectPeer_EmitsSignals) {
    QSignalSpy spyPeerChanged(viewModel.get(), &ChatViewModel::peerChanged);
    
    viewModel->setCurrentPeer(testPeerId, testPeerName);
    
    EXPECT_EQ(spyPeerChanged.count(), 1);
    
    auto arguments = spyPeerChanged.takeFirst();
    EXPECT_EQ(arguments.at(0).toString(), testPeerId);
    EXPECT_EQ(arguments.at(1).toString(), testPeerName);
}

// ========== 消息发送测试 ==========

TEST_F(ChatViewModelTest, SendMessage_WithoutPeer_ShouldFail) {
    viewModel->sendMessage("Hello");
    
    auto messages = viewModel->getMessages();
    EXPECT_TRUE(messages.isEmpty());
}

TEST_F(ChatViewModelTest, SendEmptyMessage_ShouldBeRejected) {
    viewModel->setCurrentPeer(testPeerId, testPeerName);
    viewModel->sendMessage("");
    
    auto messages = viewModel->getMessages();
    EXPECT_TRUE(messages.isEmpty());
}

TEST_F(ChatViewModelTest, SendValidMessage_WithEchoBot) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    
    QSignalSpy spyMessagesUpdated(viewModel.get(), &ChatViewModel::messagesUpdated);
    
    QString testContent = "Test message";
    viewModel->sendMessage(testContent);
    
    QTest::qWait(100);
    
    EXPECT_GT(spyMessagesUpdated.count(), 0);
    
    auto messages = viewModel->getMessages();
    EXPECT_GT(messages.size(), 0);
    
    bool foundMessage = false;
    for (const auto& msg : messages) {
        if (msg.content() == testContent) {
            foundMessage = true;
            break;
        }
    }
    EXPECT_TRUE(foundMessage);
}

// ========== 消息接收测试 ==========

TEST_F(ChatViewModelTest, ReceiveMessage_UpdatesMessageList) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    
    QSignalSpy spyMessagesUpdated(viewModel.get(), &ChatViewModel::messagesUpdated);
    
    viewModel->sendMessage("Hello Echo");
    
    EXPECT_TRUE(spyMessagesUpdated.wait(1000));
    
    auto messages = viewModel->getMessages();
    EXPECT_GE(messages.size(), 2);
}

// ========== 消息历史测试 ==========

TEST_F(ChatViewModelTest, SwitchPeer_LoadsHistory) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    viewModel->sendMessage("First message");
    QTest::qWait(100);
    
    viewModel->setCurrentPeer(testPeerId, testPeerName);
    auto messages1 = viewModel->getMessages();
    EXPECT_TRUE(messages1.isEmpty());
    
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    auto messages2 = viewModel->getMessages();
    
    EXPECT_GT(messages2.size(), 0);
}

TEST_F(ChatViewModelTest, ClearHistory_RemovesMessages) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    
    viewModel->sendMessage("Test message");
    QTest::qWait(100);
    
    viewModel->clearHistory();
    
    auto messages = viewModel->getMessages();
    EXPECT_TRUE(messages.isEmpty());
}

// ========== 消息格式化测试 ==========

TEST_F(ChatViewModelTest, MessageTimestamp_IsValid) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    
    viewModel->sendMessage("Test");
    QTest::qWait(100);
    
    auto messages = viewModel->getMessages();
    ASSERT_GT(messages.size(), 0);
    
    auto firstMsg = messages.first();
    EXPECT_TRUE(firstMsg.timestamp().isValid());
}

// ========== 性能测试 ==========

TEST_F(ChatViewModelTest, Performance_MultipleMessages) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < 20; ++i) {
        viewModel->sendMessage(QString("Message %1").arg(i));
    }
    
    QTest::qWait(500);
    
    auto endTime = QDateTime::currentMSecsSinceEpoch();
    auto duration = endTime - startTime;
    
    EXPECT_LT(duration, 1000);
    
    auto messages = viewModel->getMessages();
    EXPECT_GE(messages.size(), 20);
}

TEST_F(ChatViewModelTest, Performance_FrequentPeerSwitch) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < 50; ++i) {
        if (i % 2 == 0) {
            viewModel->setCurrentPeer(echoBotId, "Echo Bot");
        } else {
            viewModel->setCurrentPeer(testPeerId, testPeerName);
        }
    }
    
    auto endTime = QDateTime::currentMSecsSinceEpoch();
    auto duration = endTime - startTime;
    
    EXPECT_LT(duration, 300);
}

// ========== 边界条件测试 ==========

TEST_F(ChatViewModelTest, VeryLongMessage_HandledCorrectly) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    
    QString longMessage(5000, 'A');
    
    viewModel->sendMessage(longMessage);
    QTest::qWait(100);
    
    auto messages = viewModel->getMessages();
    EXPECT_GT(messages.size(), 0);
}

TEST_F(ChatViewModelTest, SpecialCharacters_HandledCorrectly) {
    QString echoBotId = services::LocalEchoService::getEchoBotId();
    viewModel->setCurrentPeer(echoBotId, "Echo Bot");
    
    QString specialMsg = "Test 中文 🚀 \n\t Special!@#$%";
    
    viewModel->sendMessage(specialMsg);
    QTest::qWait(100);
    
    auto messages = viewModel->getMessages();
    ASSERT_GT(messages.size(), 0);
    
    bool found = false;
    for (const auto& msg : messages) {
        if (msg.content() == specialMsg) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @brief 主函数
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
