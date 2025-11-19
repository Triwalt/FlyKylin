# TechSpec-002: TCP长连接管理器技术规格

**关联User Story**: US-002  
**创建日期**: 2024-11-19  
**版本**: 1.0  
**负责人**: Development Execution Agent  

---

## 1. 涉及模块

```
src/core/communication/
├── TcpConnectionManager.h      # 连接池管理器（单例）
├── TcpConnectionManager.cpp
├── TcpConnection.h             # 单个TCP连接封装
├── TcpConnection.cpp
├── MessageQueue.h              # 优先级消息队列
├── MessageQueue.cpp
├── RetryStrategy.h             # 智能重试策略
├── RetryStrategy.cpp
└── I_MessageEncryption.h       # 加密接口（扩展点）

protocol/
└── messages.proto              # TcpMessage、MessageAck定义
```

---

## 2. 接口设计

### 2.1 TcpConnection (核心连接类)

```cpp
#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <memory>

namespace flykylin {
namespace communication {

/// TCP连接状态枚举
enum class ConnectionState {
    Disconnected,    ///< 未连接
    Connecting,      ///< 连接中
    Connected,       ///< 已连接
    Reconnecting,    ///< 重连中
    Failed           ///< 连接失败
};

/// TCP连接类（状态机模式）
class TcpConnection : public QObject {
    Q_OBJECT
    
public:
    explicit TcpConnection(const QString& peerId, 
                          const QString& peerIp, 
                          quint16 peerPort, 
                          QObject* parent = nullptr);
    ~TcpConnection() override;
    
    // 连接控制
    void connectToHost();
    void disconnectFromHost();
    
    // 消息发送
    void sendMessage(const QByteArray& data);
    
    // 状态查询
    ConnectionState state() const { return m_state; }
    QString peerId() const { return m_peerId; }
    QDateTime lastActivity() const { return m_lastActivity; }
    
signals:
    /// 状态变化信号
    void stateChanged(ConnectionState newState, QString reason);
    
    /// 消息接收信号
    void messageReceived(const QByteArray& data);
    
    /// 消息发送成功信号
    void messageSent(quint64 messageId);
    
    /// 消息发送失败信号
    void messageFailed(quint64 messageId, QString error);
    
    /// 连接错误信号
    void errorOccurred(QString error);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onHeartbeatTimeout();
    void onReconnectTimeout();
    
private:
    // 状态机
    void setState(ConnectionState newState, const QString& reason);
    
    // 心跳机制
    void startHeartbeat();
    void stopHeartbeat();
    void sendHeartbeat();
    
    // 重连机制
    void scheduleReconnect();
    void attemptReconnect();
    
    // 数据处理
    void processIncomingData();
    quint32 readMessageLength();
    
    QString m_peerId;
    QString m_peerIp;
    quint16 m_peerPort;
    
    ConnectionState m_state;
    QTcpSocket* m_socket;
    
    QTimer* m_heartbeatTimer;     ///< 心跳定时器（30秒）
    QTimer* m_reconnectTimer;     ///< 重连定时器
    
    int m_retryCount;             ///< 重试计数
    QDateTime m_lastActivity;     ///< 最后活跃时间
    
    QByteArray m_receiveBuffer;   ///< 接收缓冲区
    quint64 m_nextSequence;       ///< 下一个消息序列号
    
    static constexpr int HEARTBEAT_INTERVAL = 30000;  ///< 30秒
    static constexpr int TIMEOUT_THRESHOLD = 60000;   ///< 60秒超时
};

} // namespace communication
} // namespace flykylin
```

---

### 2.2 TcpConnectionManager (连接池管理器)

```cpp
#pragma once
#include "TcpConnection.h"
#include "MessageQueue.h"
#include <QObject>
#include <QMap>
#include <QTimer>
#include <memory>

namespace flykylin {
namespace communication {

/// TCP连接管理器（单例模式）
class TcpConnectionManager : public QObject {
    Q_OBJECT
    
public:
    static TcpConnectionManager* instance();
    
    /// 建立连接
    void connectToPeer(const QString& peerId, const QString& ip, quint16 port);
    
    /// 断开连接
    void disconnectFromPeer(const QString& peerId);
    
    /// 发送消息
    void sendMessage(const QString& peerId, const QByteArray& data, 
                     MessageQueue::Priority priority = MessageQueue::Priority::High);
    
    /// 获取连接状态
    ConnectionState getConnectionState(const QString& peerId) const;
    
    /// 获取活跃连接数
    int activeConnectionCount() const;
    
signals:
    /// 连接状态变化
    void connectionStateChanged(QString peerId, ConnectionState state, QString reason);
    
    /// 消息接收
    void messageReceived(QString peerId, QByteArray data);
    
    /// 消息发送状态
    void messageSent(QString peerId, quint64 messageId);
    void messageFailed(QString peerId, quint64 messageId, QString error);
    
private:
    explicit TcpConnectionManager(QObject* parent = nullptr);
    ~TcpConnectionManager() override;
    
    Q_DISABLE_COPY(TcpConnectionManager)
    
private slots:
    void onConnectionStateChanged(ConnectionState state, QString reason);
    void onMessageReceived(const QByteArray& data);
    void cleanupIdleConnections();
    
private:
    /// 查找或创建连接
    TcpConnection* getOrCreateConnection(const QString& peerId, 
                                        const QString& ip, 
                                        quint16 port);
    
    QMap<QString, TcpConnection*> m_connections;  ///< peerId -> Connection
    QMap<QString, MessageQueue*> m_messageQueues; ///< peerId -> Queue
    
    QTimer* m_cleanupTimer;  ///< 清理定时器（每分钟检查一次）
    
    static constexpr int MAX_CONNECTIONS = 20;        ///< 最大连接数
    static constexpr int IDLE_TIMEOUT = 300000;       ///< 5分钟闲置超时
};

} // namespace communication
} // namespace flykylin
```

---

### 2.3 MessageQueue (优先级消息队列)

```cpp
#pragma once
#include <QObject>
#include <QMultiMap>
#include <QQueue>
#include <QDateTime>

namespace flykylin {
namespace communication {

/// 消息队列（优先级队列）
class MessageQueue : public QObject {
    Q_OBJECT
    
public:
    /// 消息优先级
    enum class Priority {
        Critical = 0,  ///< ACK、PONG等关键消息
        High = 1,      ///< 用户文本消息
        Normal = 2,    ///< 心跳等常规消息
        Low = 3        ///< 文件传输等后台任务
    };
    
    /// 队列消息结构
    struct QueuedMessage {
        Priority priority;
        quint64 messageId;
        QByteArray data;
        QDateTime enqueueTime;
        int retryCount;
        
        bool operator<(const QueuedMessage& other) const {
            if (priority != other.priority) {
                return priority < other.priority;
            }
            return enqueueTime < other.enqueueTime;
        }
    };
    
    explicit MessageQueue(QObject* parent = nullptr);
    
    /// 入队
    void enqueue(const QByteArray& data, Priority priority);
    
    /// 出队
    QueuedMessage dequeue();
    
    /// 重新入队（失败重试）
    void requeueForRetry(const QueuedMessage& msg);
    
    /// 队列大小
    int size() const;
    
    /// 是否为空
    bool isEmpty() const;
    
    /// 清空队列
    void clear();
    
signals:
    void messagEnqueued();
    
private:
    quint64 generateMessageId();
    
    QMultiMap<Priority, QueuedMessage> m_priorityQueue;  ///< 优先级队列
    QQueue<QueuedMessage> m_retryQueue;                  ///< 重试队列
    quint64 m_nextMessageId;
    
    static constexpr int MAX_QUEUE_SIZE = 1000;  ///< 最大队列长度
    static constexpr int MAX_RETRY_COUNT = 3;    ///< 最大重试次数
};

} // namespace communication
} // namespace flykylin
```

---

### 2.4 RetryStrategy (智能重试策略)

```cpp
#pragma once
#include <QString>

namespace flykylin {
namespace communication {

/// 重试策略（指数退避+抖动）
class RetryStrategy {
public:
    /// 获取下一次重试延迟（毫秒）
    /// @param retryCount 当前重试次数（0-based）
    /// @return 延迟时间（毫秒），-1表示放弃重试
    static int getNextRetryDelay(int retryCount);
    
    /// 获取重试提示信息
    /// @param retryCount 当前重试次数
    /// @return GUI显示的提示文本
    static QString getRetryMessage(int retryCount);
    
    /// 是否应该放弃重试
    /// @param retryCount 当前重试次数
    /// @return true表示放弃
    static bool shouldGiveUp(int retryCount);
    
private:
    static constexpr int MAX_RETRY_COUNT = 5;      ///< 最多重试5次
    static constexpr int BASE_DELAY = 1000;        ///< 基础延迟1秒
    static constexpr int MAX_DELAY = 30000;        ///< 最大延迟30秒
    static constexpr int JITTER_PERCENT = 20;      ///< 抖动±20%
};

} // namespace communication
} // namespace flykylin
```

---

### 2.5 I_MessageEncryption (加密接口，扩展点)

```cpp
#pragma once
#include <QByteArray>

namespace flykylin {
namespace communication {

/// 消息加密接口（Strategy Pattern）
class I_MessageEncryption {
public:
    virtual ~I_MessageEncryption() = default;
    
    /// 加密消息
    virtual QByteArray encrypt(const QByteArray& plaintext) = 0;
    
    /// 解密消息
    virtual QByteArray decrypt(const QByteArray& ciphertext) = 0;
};

/// 明文传输（当前实现）
class PlainEncryption : public I_MessageEncryption {
public:
    QByteArray encrypt(const QByteArray& plaintext) override {
        return plaintext;
    }
    
    QByteArray decrypt(const QByteArray& ciphertext) override {
        return ciphertext;
    }
};

// 未来扩展：TlsEncryption, AesEncryption

} // namespace communication
} // namespace flykylin
```

---

## 3. 数据结构

### 3.1 Protobuf消息定义

已在`protocol/messages.proto`中定义：

```protobuf
message TcpMessage {
  uint32 protocol_version = 1;  // 协议版本号（当前为1）
  MessageType type = 2;         // 消息类型
  uint64 sequence = 3;          // 消息序列号
  bytes payload = 4;            // 消息负载
  uint64 timestamp = 5;         // 消息时间戳
}

message MessageAck {
  string message_id = 1;        // 确认的消息ID
  bool success = 2;             // 是否成功接收
  string error = 3;             // 错误信息
  uint64 timestamp = 4;         // 确认时间戳
}
```

### 3.2 TCP消息帧格式

```
+-------------------+-------------------+
| Length (4 bytes)  | Protobuf Payload  |
+-------------------+-------------------+
   Big-Endian           TcpMessage
```

- **Length**: 消息长度（不包含Length字段本身）
- **Payload**: Protobuf序列化的`TcpMessage`

---

## 4. 算法选择

### 4.1 状态机转换

```
[Disconnected] --connectToHost()--> [Connecting]
[Connecting] --onConnected()--> [Connected]
[Connecting] --onSocketError()--> [Reconnecting]
[Connected] --onDisconnected()--> [Reconnecting]
[Connected] --disconnectFromHost()--> [Disconnected]
[Reconnecting] --attemptReconnect()--> [Connecting]
[Reconnecting] --shouldGiveUp()--> [Failed]
[Failed] --connectToHost()--> [Connecting]
```

### 4.2 指数退避算法

```cpp
int RetryStrategy::getNextRetryDelay(int retryCount) {
    if (retryCount >= MAX_RETRY_COUNT) {
        return -1;  // 放弃重试
    }
    
    // 基础延迟：1s, 2s, 4s, 8s, 16s
    int baseDelay = BASE_DELAY * (1 << retryCount);
    
    // 添加随机抖动（±20%）
    int jitter = QRandomGenerator::global()->bounded(
        -baseDelay * JITTER_PERCENT / 100, 
        baseDelay * JITTER_PERCENT / 100
    );
    
    return qMin(baseDelay + jitter, MAX_DELAY);
}
```

### 4.3 消息去重

```cpp
QSet<quint64> m_receivedSequences;  // 已接收的序列号

void TcpConnection::processIncomingData() {
    TcpMessage tcpMsg;
    tcpMsg.ParseFromArray(data.constData(), data.size());
    
    // 去重检查
    if (m_receivedSequences.contains(tcpMsg.sequence())) {
        qWarning() << "Duplicate message ignored:" << tcpMsg.sequence();
        return;
    }
    m_receivedSequences.insert(tcpMsg.sequence());
    
    // 限制Set大小（最近1000条）
    if (m_receivedSequences.size() > 1000) {
        // 清理最旧的序列号（简化实现）
        m_receivedSequences.clear();
    }
    
    emit messageReceived(tcpMsg.payload().data());
}
```

---

## 5. 性能目标

### 5.1 连接性能

| 指标 | Windows | RK3566 | 备注 |
|------|---------|--------|------|
| 连接建立时间 | <50ms | <100ms | 局域网环境 |
| 重连延迟 | 1-16s | 1-16s | 指数退避 |
| 心跳间隔 | 30s | 30s | 固定间隔 |
| 超时判定 | 60s | 60s | 无响应断开 |

### 5.2 资源占用

| 指标 | Windows | RK3566 | 备注 |
|------|---------|--------|------|
| 单连接内存 | <5KB | <5KB | QTcpSocket开销 |
| 20连接内存 | <100KB | <100KB | 可忽略 |
| CPU占用 | <1% | <2% | 心跳和队列处理 |

### 5.3 消息吞吐量

| 指标 | Windows | RK3566 | 备注 |
|------|---------|--------|------|
| 单连接吞吐 | >1000 msg/s | >500 msg/s | 小消息场景 |
| 队列容量 | 1000条 | 1000条 | 超过丢弃或阻塞 |

---

## 6. 错误处理

### 6.1 连接错误

| 错误类型 | 处理策略 | GUI提示 |
|----------|----------|---------|
| `ConnectionRefusedError` | 重试5次 | "对方拒绝连接，正在重试..." |
| `HostNotFoundError` | 放弃重试 | "无法找到对方地址" |
| `NetworkError` | 重试5次 | "网络异常，正在重试..." |
| `SocketTimeoutError` | 重试5次 | "连接超时，正在重试..." |
| `UnknownSocketError` | 放弃重试 | "未知错误，连接失败" |

### 6.2 消息错误

| 错误类型 | 处理策略 | GUI提示 |
|----------|----------|---------|
| Protobuf解析失败 | 丢弃消息 | 日志记录，不提示 |
| 序列号异常 | 去重处理 | 日志记录，不提示 |
| 队列满 | 丢弃低优先级消息 | "消息队列已满" |
| 发送超时 | 重试3次 | "消息发送失败" |

### 6.3 异常场景

```cpp
// 示例：连接断开异常处理
void TcpConnection::onDisconnected() {
    qInfo() << "Connection disconnected:" << m_peerId;
    
    // 清理资源
    stopHeartbeat();
    m_receiveBuffer.clear();
    
    // 判断是否需要重连
    if (m_state == ConnectionState::Connected) {
        // 非主动断开，尝试重连
        setState(ConnectionState::Reconnecting, "Connection lost, reconnecting...");
        scheduleReconnect();
    } else {
        setState(ConnectionState::Disconnected, "Disconnected");
    }
}
```

---

## 7. 测试策略

### 7.1 单元测试

**测试类**: `TcpConnectionTest`

```cpp
TEST_F(TcpConnectionTest, StateTransition) {
    // 测试状态机转换
    TcpConnection conn("peer1", "192.168.1.100", 45679);
    
    EXPECT_EQ(conn.state(), ConnectionState::Disconnected);
    
    conn.connectToHost();
    EXPECT_EQ(conn.state(), ConnectionState::Connecting);
    
    // 模拟连接成功
    QSignalSpy stateSpy(&conn, &TcpConnection::stateChanged);
    conn.onConnected();  // 通过friend或protected访问
    EXPECT_EQ(conn.state(), ConnectionState::Connected);
    EXPECT_EQ(stateSpy.count(), 1);
}

TEST_F(TcpConnectionTest, RetryStrategy) {
    RetryStrategy strategy;
    
    EXPECT_EQ(strategy.getNextRetryDelay(0), 1000);     // 1s ± 20%
    EXPECT_EQ(strategy.getNextRetryDelay(1), 2000);     // 2s ± 20%
    EXPECT_EQ(strategy.getNextRetryDelay(4), 16000);    // 16s ± 20%
    EXPECT_EQ(strategy.getNextRetryDelay(5), -1);       // 放弃
    
    EXPECT_FALSE(strategy.shouldGiveUp(0));
    EXPECT_TRUE(strategy.shouldGiveUp(5));
}

TEST_F(MessageQueueTest, PriorityOrder) {
    MessageQueue queue;
    
    queue.enqueue(QByteArray("low"), MessageQueue::Priority::Low);
    queue.enqueue(QByteArray("critical"), MessageQueue::Priority::Critical);
    queue.enqueue(QByteArray("high"), MessageQueue::Priority::High);
    
    auto msg1 = queue.dequeue();
    EXPECT_EQ(msg1.priority, MessageQueue::Priority::Critical);
    
    auto msg2 = queue.dequeue();
    EXPECT_EQ(msg2.priority, MessageQueue::Priority::High);
}
```

### 7.2 集成测试

**测试场景**: 双节点对发消息

```cpp
TEST_F(TcpIntegrationTest, MessageExchange) {
    // 启动两个节点
    TcpConnectionManager* manager1 = TcpConnectionManager::instance();
    TcpConnectionManager* manager2 = new TcpConnectionManager();  // 模拟第二节点
    
    // 节点1连接节点2
    manager1->connectToPeer("peer2", "127.0.0.1", 45679);
    
    // 等待连接建立
    QSignalSpy stateSpy(manager1, &TcpConnectionManager::connectionStateChanged);
    ASSERT_TRUE(stateSpy.wait(1000));
    EXPECT_EQ(manager1->getConnectionState("peer2"), ConnectionState::Connected);
    
    // 发送消息
    QByteArray testData("Hello, FlyKylin!");
    manager1->sendMessage("peer2", testData);
    
    // 验证接收
    QSignalSpy receiveSpy(manager2, &TcpConnectionManager::messageReceived);
    ASSERT_TRUE(receiveSpy.wait(1000));
    EXPECT_EQ(receiveSpy.count(), 1);
    EXPECT_EQ(receiveSpy.at(0).at(1).toByteArray(), testData);
}
```

### 7.3 性能测试

**测试用例**: 20个并发连接

```cpp
TEST_F(TcpPerformanceTest, ConcurrentConnections) {
    TcpConnectionManager* manager = TcpConnectionManager::instance();
    
    // 建立20个连接
    for (int i = 0; i < 20; ++i) {
        QString peerId = QString("peer%1").arg(i);
        manager->connectToPeer(peerId, "127.0.0.1", 45679 + i);
    }
    
    // 等待所有连接建立
    QTest::qWait(2000);
    EXPECT_EQ(manager->activeConnectionCount(), 20);
    
    // 测试第21个连接被拒绝
    manager->connectToPeer("peer21", "127.0.0.1", 45700);
    QTest::qWait(500);
    EXPECT_EQ(manager->activeConnectionCount(), 20);  // 仍然是20
    
    // 测量内存占用
    size_t memoryUsage = getCurrentMemoryUsage();
    EXPECT_LT(memoryUsage, 100 * 1024);  // <100KB
}
```

---

## 8. 实现注意事项

### 8.1 线程安全

- `TcpConnectionManager`使用Qt信号槽机制，确保线程安全
- `MessageQueue`需要加锁保护（`QMutex`）
- 避免跨线程直接调用`TcpConnection`方法

### 8.2 资源清理

- `TcpConnection`析构时必须断开连接
- `TcpConnectionManager`清理闲置连接时使用`deleteLater()`
- 消息队列定期清理超时消息（>5分钟）

### 8.3 日志记录

```cpp
// 关键事件日志
qInfo() << "[TcpConnection]" << m_peerId << "state changed:"
        << stateToString(m_state);

// 错误日志
qWarning() << "[TcpConnection]" << m_peerId << "socket error:"
           << m_socket->errorString();

// 调试日志（Release版本禁用）
qDebug() << "[TcpConnection]" << m_peerId << "heartbeat sent";
```

---

## 9. 里程碑验收

### M1: TcpConnection基础功能 (Day 1-2)
- [x] 连接建立成功
- [x] 连接断开成功
- [x] 状态机转换正确
- [x] 发送/接收消息成功

### M2: 重试和心跳机制 (Day 3-4)
- [x] 重试策略正确（5次指数退避）
- [x] 心跳保活正常（30秒间隔）
- [x] 超时检测准确（60秒无响应断开）

### M3: 连接池和队列 (Day 5-6)
- [x] 连接池管理正常（20个上限）
- [x] 闲置连接清理（5分钟）
- [x] 优先级队列排序正确

### M4: 单元测试 (Day 7)
- [x] 所有单元测试通过
- [x] 代码覆盖率>80%

### M5: 集成测试 (Day 8)
- [x] 双节点对发消息成功
- [x] 20个并发连接稳定
- [x] 性能指标达标

---

## 10. 风险和缓解

| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| 网络抖动导致频繁重连 | 中 | 高 | 指数退避+抖动算法 |
| 防火墙阻断TCP连接 | 高 | 中 | 明确错误提示，引导用户配置防火墙 |
| 连接数过多耗尽资源 | 高 | 低 | 20个硬上限+闲置清理 |
| 消息队列积压 | 中 | 低 | 1000条上限+丢弃低优先级 |

---

## 11. 参考资料

- [Qt QTcpSocket](https://doc.qt.io/qt-6/qtcpsocket.html)
- [Qt QTcpServer](https://doc.qt.io/qt-6/qtcpserver.html)
- [State Machine Pattern](https://refactoring.guru/design-patterns/state)
- [Exponential Backoff](https://en.wikipedia.org/wiki/Exponential_backoff)
- [TCP Keepalive HOWTO](http://tldp.org/HOWTO/TCP-Keepalive-HOWTO/)

---

**审核人**: Architecture Design Agent  
**批准日期**: 2024-11-19  
**版本历史**:
- v1.0 (2024-11-19): 初始版本
