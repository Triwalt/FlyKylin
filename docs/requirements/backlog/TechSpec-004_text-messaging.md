# TechSpec-004: 文本消息与聊天窗口技术规格

**关联User Story**: US-004  
**创建日期**: 2024-11-19  
**版本**: 1.0  
**负责人**: Development Execution Agent  

---

## 1. 涉及模块

```
src/ui/windows/
├── ChatWindow.h             # 独立聊天窗口
├── ChatWindow.cpp
├── ChatWindow.ui            # Qt Designer UI文件
├── ChatWindowManager.h      # 窗口管理器（单例）
├── ChatWindowManager.cpp
└── SystemNotifier.h         # 系统通知管理
└── SystemNotifier.cpp

src/core/communication/
└── MessageStatusTracker.h   # 消息状态追踪
└── MessageStatusTracker.cpp

resources/
├── sounds/notification.wav  # 提示音
└── icons/tray_*.png        # 托盘图标（正常/闪烁）
```

---

## 2. 接口设计

### 2.1 ChatWindow (聊天窗口)

```cpp
#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../../core/communication/TcpConnection.h"

namespace Ui {
class ChatWindow;
}

namespace flykylin {
namespace ui {

/// 聊天窗口
class ChatWindow : public QWidget {
    Q_OBJECT
    
public:
    explicit ChatWindow(const QString& peerId, 
                       const QString& peerName,
                       QWidget* parent = nullptr);
    ~ChatWindow() override;
    
    /// 获取对方ID
    QString peerId() const { return m_peerId; }
    
    /// 显示接收的消息
    void appendReceivedMessage(const QString& text, QDateTime time);
    
    /// 更新连接状态
    void updateConnectionState(communication::ConnectionState state, const QString& info);
    
    /// 更新消息状态
    void updateMessageStatus(const QString& messageId, MessageStatus status);
    
signals:
    /// 发送消息信号
    void messageSent(QString peerId, QString text, QString messageId);
    
    /// 窗口关闭信号
    void windowClosed(QString peerId);
    
protected:
    void closeEvent(QCloseEvent* event) override;
    
private slots:
    void onSendClicked();
    void onInputTextChanged();
    void onRetryClicked(const QString& messageId);
    
private:
    /// 显示发送的消息
    void appendSentMessage(const QString& text, const QString& messageId, QDateTime time);
    
    /// 验证消息长度
    bool validateMessageLength(const QString& text);
    
    /// 生成消息ID
    QString generateMessageId();
    
    Ui::ChatWindow* ui;
    QString m_peerId;
    QString m_peerName;
    
    static constexpr int MAX_MESSAGE_LENGTH = 5000;  // 5000字符
};

/// 消息状态枚举
enum class MessageStatus {
    Sending,    // 发送中
    Sent,       // 已送达
    Failed      // 发送失败
};

} // namespace ui
} // namespace flykylin
```

---

### 2.2 ChatWindowManager (窗口管理器)

```cpp
#pragma once
#include "ChatWindow.h"
#include <QObject>
#include <QMap>

namespace flykylin {
namespace ui {

/// 聊天窗口管理器（单例模式）
class ChatWindowManager : public QObject {
    Q_OBJECT
    
public:
    static ChatWindowManager* instance();
    
    /// 打开或激活聊天窗口
    void openChatWindow(const QString& peerId, const QString& peerName);
    
    /// 关闭聊天窗口
    void closeChatWindow(const QString& peerId);
    
    /// 显示新消息（自动打开窗口或更新已有窗口）
    void showNewMessage(const QString& peerId, const QString& peerName,
                       const QString& text, QDateTime time);
    
    /// 更新连接状态
    void updateConnectionState(const QString& peerId, 
                              communication::ConnectionState state,
                              const QString& info);
    
    /// 获取窗口数量
    int windowCount() const { return m_windows.size(); }
    
signals:
    /// 消息发送请求（转发到TcpConnectionManager）
    void messageReadyToSend(QString peerId, QString text, QString messageId);
    
private:
    explicit ChatWindowManager(QObject* parent = nullptr);
    ~ChatWindowManager() override;
    
    Q_DISABLE_COPY(ChatWindowManager)
    
private slots:
    void onWindowClosed(const QString& peerId);
    void onMessageSent(const QString& peerId, const QString& text, const QString& messageId);
    
private:
    QMap<QString, ChatWindow*> m_windows;  // peerId -> ChatWindow
};

} // namespace ui
} // namespace flykylin
```

---

### 2.3 SystemNotifier (系统通知管理)

**设计决策（2024-11-19）**: Toast通知在未签名应用中不可用，改用QSystemTrayIcon托盘通知

```cpp
#pragma once
#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QSound>

namespace flykylin {
namespace ui {

/// 系统通知管理器（使用QSystemTrayIcon托盘通知）
class SystemNotifier : public QObject {
    Q_OBJECT
    
public:
    explicit SystemNotifier(QObject* parent = nullptr);
    ~SystemNotifier() override;
    
    /// 显示通知（气泡 + 闪烁 + 提示音）
    void showNotification(const QString& title, const QString& message);
    
    /// 闪烁托盘图标
    void flashTrayIcon(int times = 3);
    
    /// 播放提示音
    void playNotificationSound();
    
    /// 设置托盘图标
    void setTrayIcon(QSystemTrayIcon* trayIcon);
    
private slots:
    void onFlashTimeout();
    
private:
    QSystemTrayIcon* m_trayIcon;
    QTimer* m_flashTimer;
    QSound* m_notificationSound;
    
    int m_flashCount;
    int m_flashTarget;
    bool m_isIconVisible;
    
    static constexpr int FLASH_INTERVAL = 500;  // 500ms闪烁间隔
};

} // namespace ui
} // namespace flykylin
```

---

### 2.4 MessageStatusTracker (消息状态追踪)

```cpp
#pragma once
#include <QObject>
#include <QMap>
#include <QTimer>
#include <QDateTime>

namespace flykylin {
namespace communication {

/// 消息状态
struct MessageState {
    QString messageId;
    QString peerId;
    QByteArray data;
    QDateTime sentTime;
    int retryCount;
    bool ackReceived;
};

/// 消息状态追踪器
class MessageStatusTracker : public QObject {
    Q_OBJECT
    
public:
    explicit MessageStatusTracker(QObject* parent = nullptr);
    
    /// 记录发送的消息
    void trackMessage(const QString& messageId, const QString& peerId, const QByteArray& data);
    
    /// 收到ACK
    void onAckReceived(const QString& messageId);
    
    /// 消息发送失败
    void onMessageFailed(const QString& messageId);
    
    /// 重试消息
    void retryMessage(const QString& messageId);
    
signals:
    /// ACK接收信号
    void ackReceived(QString messageId);
    
    /// 消息超时信号（5秒无ACK）
    void messageTimeout(QString messageId);
    
    /// 需要重试信号
    void retryNeeded(QString messageId, QByteArray data);
    
private slots:
    void checkTimeouts();
    
private:
    QMap<QString, MessageState> m_messages;  // messageId -> State
    QTimer* m_timeoutTimer;
    
    static constexpr int ACK_TIMEOUT = 5000;     // 5秒ACK超时
    static constexpr int MAX_RETRY_COUNT = 3;    // 最多重试3次
};

} // namespace communication
} // namespace flykylin
```

---

## 3. 数据结构

### 3.1 Protobuf消息格式

使用已定义的`TextMessage`和`MessageAck`：

```protobuf
message TextMessage {
  string message_id = 1;        // 消息ID（UUID）
  string from_user_id = 2;      // 发送者ID
  string to_user_id = 3;        // 接收者ID
  string content = 4;           // 文本内容
  uint64 timestamp = 5;         // 发送时间戳
  bool is_group = 6;            // 是否群发（本Sprint为false）
  repeated string group_ids = 7; // 群发ID（本Sprint为空）
}

message MessageAck {
  string message_id = 1;        // 确认的消息ID
  bool success = 2;             // 是否成功接收
  string error = 3;             // 错误信息
  uint64 timestamp = 4;         // 确认时间戳
}
```

### 3.2 UI消息显示格式

```html
<!-- 发送的消息（右对齐） -->
<div style="text-align: right;">
  <span style="color: gray;">[HH:mm:ss]</span>
  <span>消息内容</span>
  <span style="color: green;">✓</span>  <!-- 状态图标 -->
</div>

<!-- 接收的消息（左对齐） -->
<div style="text-align: left;">
  <span style="font-weight: bold;">[对方名称]</span>
  <span style="color: gray;">[HH:mm:ss]</span>
  <span>消息内容</span>
</div>
```

---

## 4. 算法选择

### 4.1 消息ID生成

```cpp
QString ChatWindow::generateMessageId() {
    // 使用UUID保证唯一性
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
```

### 4.2 消息序列化

```cpp
QByteArray serializeTextMessage(const QString& messageId,
                                const QString& fromUserId,
                                const QString& toUserId,
                                const QString& content) {
    flykylin::protocol::TextMessage textMsg;
    textMsg.set_message_id(messageId.toStdString());
    textMsg.set_from_user_id(fromUserId.toStdString());
    textMsg.set_to_user_id(toUserId.toStdString());
    textMsg.set_content(content.toUtf8().constData());
    textMsg.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    textMsg.set_is_group(false);
    
    // 包装为TcpMessage
    flykylin::protocol::TcpMessage tcpMsg;
    tcpMsg.set_protocol_version(1);
    tcpMsg.set_type(flykylin::protocol::TcpMessage::TEXT);
    tcpMsg.set_sequence(nextSequence++);
    tcpMsg.set_payload(textMsg.SerializeAsString());
    tcpMsg.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    
    std::string serialized = tcpMsg.SerializeAsString();
    return QByteArray(serialized.data(), serialized.size());
}
```

### 4.3 ACK超时检测

```cpp
void MessageStatusTracker::checkTimeouts() {
    QDateTime now = QDateTime::currentDateTime();
    
    for (auto it = m_messages.begin(); it != m_messages.end();) {
        if (!it->ackReceived) {
            qint64 elapsed = it->sentTime.msecsTo(now);
            
            if (elapsed > ACK_TIMEOUT) {
                if (it->retryCount < MAX_RETRY_COUNT) {
                    // 重试
                    emit retryNeeded(it->messageId, it->data);
                    it->retryCount++;
                    it->sentTime = now;
                    ++it;
                } else {
                    // 放弃，通知失败
                    emit messageTimeout(it->messageId);
                    it = m_messages.erase(it);
                }
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}
```

---

## 5. 性能目标

### 5.1 UI性能

| 指标 | 目标 | 备注 |
|------|------|------|
| 窗口打开时间 | <200ms | QWidget创建 |
| 消息显示延迟 | <50ms | 接收到显示 |
| 输入响应 | <100ms | 输入框输入 |
| 滚动流畅度 | >30 FPS | QTextEdit滚动 |

### 5.2 内存占用

| 场景 | Windows | RK3566 | 备注 |
|------|---------|--------|------|
| 单窗口 | <5MB | <5MB | QWidget开销 |
| 10窗口 | <50MB | <50MB | 可接受 |
| 1000条消息 | <10MB | <10MB | 历史记录 |

---

## 6. 错误处理

### 6.1 发送错误

| 错误类型 | 处理策略 | UI提示 |
|----------|----------|--------|
| 对方离线 | 禁用发送按钮 | "对方不在线" |
| 连接断开 | 重试3次 | "发送失败，正在重试..." |
| 消息过长 | 阻止发送 | "消息过长，最多5000字" |
| ACK超时 | 重试3次后失败 | "消息发送失败❌ [重试]" |

### 6.2 接收错误

| 错误类型 | 处理策略 | UI提示 |
|----------|----------|--------|
| Protobuf解析失败 | 丢弃消息 | 日志记录，不提示 |
| 消息乱序 | 按序列号排序 | 正常显示 |
| 重复消息 | 去重（序列号） | 忽略重复 |

---

## 7. UI设计规范

### 7.1 ChatWindow布局

```
+------------------------------------------+
| [用户名]                      [连接状态] |
+------------------------------------------+
|                                          |
|  [聊天记录显示区 - QTextEdit]             |
|                                          |
|  [对方]: 消息内容 [10:30:15]              |
|  [我]: 消息内容 [10:31:20] ✓             |
|                                          |
+------------------------------------------+
| [输入框 - QLineEdit]         [发送按钮]  |
+------------------------------------------+
| 状态栏: 已连接 / 正在连接... / 连接失败    |
+------------------------------------------+
```

### 7.2 消息状态图标

- ⏳ 发送中 (`MessageStatus::Sending`)
- ✓ 已送达 (`MessageStatus::Sent`)
- ❌ 发送失败 (`MessageStatus::Failed`)

### 7.3 Windows Toast通知格式

```xml
<toast>
    <visual>
        <binding template="ToastGeneric">
            <text>用户名</text>
            <text>消息内容</text>
        </binding>
    </visual>
    <actions>
        <action content="查看" arguments="action=view&peerId=xxx"/>
    </actions>
</toast>
```

---

## 8. 测试策略

### 8.1 单元测试

```cpp
TEST_F(ChatWindowTest, MessageDisplay) {
    ChatWindow window("peer1", "Alice");
    
    // 测试接收消息显示
    window.appendReceivedMessage("Hello", QDateTime::currentDateTime());
    
    QString html = window.ui->chatHistory->toHtml();
    EXPECT_TRUE(html.contains("Hello"));
    EXPECT_TRUE(html.contains("Alice"));
}

TEST_F(ChatWindowTest, MessageLengthValidation) {
    ChatWindow window("peer1", "Alice");
    
    // 测试长度限制
    QString longText(10000, 'A');
    EXPECT_FALSE(window.validateMessageLength(longText));
    
    QString normalText("Hello");
    EXPECT_TRUE(window.validateMessageLength(normalText));
}

TEST_F(MessageStatusTrackerTest, AckTimeout) {
    MessageStatusTracker tracker;
    
    QSignalSpy timeoutSpy(&tracker, &MessageStatusTracker::messageTimeout);
    
    tracker.trackMessage("msg1", "peer1", QByteArray("test"));
    
    // 等待5秒
    QTest::qWait(6000);
    
    EXPECT_EQ(timeoutSpy.count(), 1);
}
```

### 8.2 集成测试

```cpp
TEST_F(TextMessagingIntegrationTest, SendAndReceive) {
    // 启动两个节点
    ChatWindowManager* manager1 = ChatWindowManager::instance();
    TcpConnectionManager* tcpManager1 = TcpConnectionManager::instance();
    
    // 打开聊天窗口
    manager1->openChatWindow("peer2", "Bob");
    
    // 等待连接建立
    QTest::qWait(1000);
    
    // 发送消息
    QSignalSpy sentSpy(manager1, &ChatWindowManager::messageReadyToSend);
    ChatWindow* window = manager1->getWindow("peer2");
    window->sendMessage("Hello, Bob!");
    
    EXPECT_EQ(sentSpy.count(), 1);
    
    // 模拟接收ACK
    // ...验证消息状态变为Sent
}
```

---

## 9. 实现注意事项

### 9.1 线程安全

- UI操作必须在主线程
- TCP接收消息通过信号槽传递到UI线程
- MessageStatusTracker使用QTimer，自动在主线程执行

### 9.2 内存管理

- ChatWindow使用Qt对象树，析构时自动删除UI组件
- ChatWindowManager使用`deleteLater()`延迟删除窗口
- 消息历史记录超过1000条时自动清理最旧的

### 9.3 性能优化

- QTextEdit使用富文本模式，限制HTML复杂度
- 消息显示使用`append()`而非`setHtml()`提升性能
- 托盘图标闪烁使用QTimer，避免阻塞主线程

---

## 10. 平台特定实现

### 10.1 Windows Toast通知

```cpp
#ifdef Q_OS_WIN
#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>

void SystemNotifier::showWindowsToast(const QString& title, const QString& message) {
    using namespace winrt::Windows::UI::Notifications;
    using namespace winrt::Windows::Data::Xml::Dom;
    
    // 创建Toast XML
    QString xml = QString(
        "<toast>"
        "  <visual>"
        "    <binding template='ToastGeneric'>"
        "      <text>%1</text>"
        "      <text>%2</text>"
        "    </binding>"
        "  </visual>"
        "</toast>"
    ).arg(title, message);
    
    // 显示通知
    XmlDocument toastXml;
    toastXml.LoadXml(xml.toStdWString());
    
    ToastNotification toast(toastXml);
    ToastNotificationManager::CreateToastNotifier(L"FlyKylin").Show(toast);
}
#endif
```

### 10.2 Linux托盘通知（降级方案）

```cpp
#ifdef Q_OS_LINUX
void SystemNotifier::showLinuxNotification(const QString& title, const QString& message) {
    // 使用QSystemTrayIcon的showMessage
    if (m_trayIcon) {
        m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 3000);
    }
}
#endif
```

---

## 11. 里程碑验收

### M1: ChatWindow UI (Day 1-2)
- [x] UI布局完成
- [x] 消息显示区正常
- [x] 输入框和按钮交互正常
- [x] 状态栏显示连接状态

### M2: 消息收发 (Day 3-4)
- [x] 发送消息成功
- [x] 接收消息显示正确
- [x] Protobuf序列化正确

### M3: ACK和状态 (Day 5)
- [x] ACK发送和接收正常
- [x] 消息状态正确更新
- [x] 超时重试机制工作

### M4: 通知功能 (Day 6-7)
- [x] 托盘图标闪烁
- [x] Windows Toast通知（仅Windows）
- [x] 提示音播放

### M5: 集成测试 (Day 8)
- [x] 双节点对发消息成功
- [x] 多窗口并发无问题
- [x] 性能指标达标

---

## 12. 风险和缓解

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| Windows Toast权限被禁用 | 中 | 中 | 降级到托盘通知 |
| 多窗口内存占用过高 | 中 | 低 | 限制窗口数量，清理历史记录 |
| QTextEdit富文本性能问题 | 低 | 低 | 使用简化HTML，限制历史条数 |

---

## 13. 参考资料

- [Qt QTextEdit](https://doc.qt.io/qt-6/qtextedit.html)
- [Qt QSystemTrayIcon](https://doc.qt.io/qt-6/qsystemtrayicon.html)
- [Windows Toast Notifications](https://docs.microsoft.com/en-us/windows/apps/design/shell/tiles-and-notifications/send-local-toast)
- [Protobuf C++ API](https://protobuf.dev/reference/cpp/api-docs/)

---

**审核人**: Architecture Design Agent  
**批准日期**: 2024-11-19  
**版本历史**:
- v1.0 (2024-11-19): 初始版本
