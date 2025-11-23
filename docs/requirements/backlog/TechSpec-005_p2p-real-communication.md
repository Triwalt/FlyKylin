# TechSpec-005: 真实P2P消息通信技术规格

**关联Story**: [US-005 真实P2P消息通信](US-005_p2p-real-communication.md)  
**创建日期**: 2024-11-20  
**负责人**: Development Execution Agent  
**状态**: 📋 设计中

---

## 📖 概述

本技术规格定义如何实现基于TCP的真实P2P消息通信，替代Echo Bot测试方案，建立可用的点对点通信能力。

**核心目标**:
1. PeerDiscovery发现节点后自动建立TCP连接
2. 实现握手认证机制
3. 消息通过TCP连接真实收发
4. 支持断线自动重连

---

## 🏗️ 架构设计

### 模块依赖关系

```
PeerDiscovery (UDP)
    ↓ peerDiscovered信号
TcpConnectionManager
    ↓ connectionEstablished信号
MessageService
    ↓ messageReceived信号
ChatViewModel
    ↓ UI更新
ChatWindow
```

### 数据流

#### 连接建立流程
```
1. PeerDiscovery → onPeerDiscovered(PeerNode)
2. TcpConnectionManager → connectToPeer(ip, port)
3. QTcpSocket → connectToHost()
4. [TCP三次握手]
5. TcpConnection → sendHandshake()
6. 对端 → receiveHandshake() → validateUUID()
7. 对端 → sendHandshakeResponse(accepted=true)
8. TcpConnection → receiveHandshakeResponse()
9. TcpConnectionManager → emit connectionEstablished(peerId)
```

#### 消息发送流程
```
1. ChatWindow → ViewModel::sendMessage(content)
2. ChatViewModel → MessageService::sendTextMessage(peerId, content)
3. MessageService → 检查peerId != echo_bot_local
4. MessageService → serializeTextMessage()
5. MessageService → TcpConnectionManager::sendMessage(peerId, data)
6. TcpConnectionManager → getConnection(peerId)
7. TcpConnection → enqueueMessage(data, priority)
8. MessageQueue → send via QTcpSocket
9. 对端 → onDataReceived() → processMessage()
10. 对端MessageService → parseTextMessage() → emit messageReceived()
```

---

## 🔌 接口设计

### 1. PeerDiscovery接口扩展

#### 新增信号
```cpp
class PeerDiscovery : public QObject {
    Q_OBJECT

signals:
    // 新增：节点发现信号
    void peerDiscovered(const core::PeerNode& node);
    
    // 现有信号保留
    void peerUpdated(QString userId);
    void peerRemoved(QString userId);
};
```

#### 实现修改
```cpp
void PeerDiscovery::processReceivedMessage(
    const QByteArray& datagram,
    const QHostAddress& senderAddress
) {
    // ...现有逻辑...
    
    if (!m_peers.contains(userId)) {
        // 新节点发现
        m_peers[userId] = node;
        emit peerDiscovered(node);  // 🆕 发射新信号
        emit peerUpdated(userId);   // 保留兼容
    }
}
```

---

### 2. TcpConnectionManager接口扩展

#### 新增方法
```cpp
class TcpConnectionManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 连接到对等节点
     * @param node 节点信息（包含IP和端口）
     * @return 连接是否成功启动
     */
    bool connectToPeer(const core::PeerNode& node);
    
    /**
     * @brief 获取连接状态
     * @param peerId 对等节点ID
     * @return 连接状态
     */
    ConnectionState getConnectionState(const QString& peerId) const;

public slots:
    /**
     * @brief 处理节点发现事件
     * @param node 发现的节点
     */
    void onPeerDiscovered(const core::PeerNode& node);
    
signals:
    /**
     * @brief 连接建立成功
     * @param peerId 对等节点ID
     */
    void connectionEstablished(const QString& peerId);
    
    /**
     * @brief 连接失败
     * @param peerId 对等节点ID
     * @param error 错误信息
     */
    void connectionFailed(const QString& peerId, const QString& error);
};
```

---

### 3. TcpConnection握手逻辑

#### 握手方法
```cpp
class TcpConnection : public QObject {
    Q_OBJECT

private:
    enum class HandshakeState {
        NotStarted,
        RequestSent,
        ResponseReceived,
        Completed,
        Failed
    };
    
    HandshakeState m_handshakeState = HandshakeState::NotStarted;
    QTimer* m_handshakeTimer = nullptr;
    static constexpr int kHandshakeTimeout = 5000; // 5秒

    /**
     * @brief 发送握手请求
     */
    void sendHandshakeRequest();
    
    /**
     * @brief 处理握手响应
     */
    void handleHandshakeResponse(const QByteArray& data);
    
    /**
     * @brief 握手超时处理
     */
    void onHandshakeTimeout();

signals:
    void handshakeCompleted();
    void handshakeFailed(const QString& error);
};
```

#### 实现示例
```cpp
void TcpConnection::sendHandshakeRequest() {
    if (m_handshakeState != HandshakeState::NotStarted) {
        return;
    }
    
    // 创建握手请求
    protocol::HandshakeRequest request;
    request.set_protocol_version("1.0");
    request.set_user_id(UserProfile::instance().userId().toStdString());
    request.set_user_name(UserProfile::instance().userName().toStdString());
    request.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    
    // 包装为TcpMessage
    protocol::TcpMessage tcpMsg;
    tcpMsg.set_type(protocol::TcpMessage::HANDSHAKE_REQUEST);
    tcpMsg.set_payload(request.SerializeAsString());
    
    // 发送
    QByteArray data(tcpMsg.ByteSizeLong(), Qt::Uninitialized);
    tcpMsg.SerializeToArray(data.data(), data.size());
    
    m_socket->write(data);
    m_handshakeState = HandshakeState::RequestSent;
    
    // 启动超时计时器
    m_handshakeTimer->start(kHandshakeTimeout);
    
    qInfo() << "[TcpConnection] Handshake request sent to" << m_peerId;
}

void TcpConnection::handleHandshakeResponse(const QByteArray& data) {
    protocol::TcpMessage tcpMsg;
    if (!tcpMsg.ParseFromArray(data.data(), data.size())) {
        emit handshakeFailed("Failed to parse handshake response");
        return;
    }
    
    protocol::HandshakeResponse response;
    if (!response.ParseFromString(tcpMsg.payload())) {
        emit handshakeFailed("Invalid handshake response payload");
        return;
    }
    
    if (!response.accepted()) {
        emit handshakeFailed(QString::fromStdString(response.error_message()));
        return;
    }
    
    // 握手成功
    m_handshakeTimer->stop();
    m_handshakeState = HandshakeState::Completed;
    emit handshakeCompleted();
    
    qInfo() << "[TcpConnection] Handshake completed with" << m_peerId;
}
```

---

## 📦 数据结构设计

### Protobuf协议扩展

#### messages.proto新增
```protobuf
// 握手请求
message HandshakeRequest {
    string protocol_version = 1;  // "1.0"
    string user_id = 2;           // UUID
    string user_name = 3;         // 用户名
    int64 timestamp = 4;          // 时间戳
}

// 握手响应
message HandshakeResponse {
    bool accepted = 1;            // 是否接受连接
    string user_id = 2;           // 对端UUID
    string user_name = 3;         // 对端用户名
    string error_message = 4;     // 错误信息（如果不接受）
}

// TcpMessage类型扩展
enum MessageType {
    // ...现有类型...
    HANDSHAKE_REQUEST = 10;
    HANDSHAKE_RESPONSE = 11;
}
```

---

## 🔄 状态机设计

### 连接状态机

```
           ┌─────────────┐
           │ Disconnected│
           └──────┬──────┘
                  │ connectToPeer()
                  ↓
           ┌─────────────┐
           │ Connecting  │ ─────→ [TCP连接失败] ──→ Error
           └──────┬──────┘
                  │ connected()
                  ↓
           ┌─────────────┐
           │ Handshaking │ ─────→ [握手超时/失败] ──→ Error
           └──────┬──────┘
                  │ handshakeCompleted()
                  ↓
           ┌─────────────┐
           │  Connected  │ ─────→ [断开] ──────────→ Disconnected
           └─────────────┘
                  │
                  │ [重连触发]
                  ↓
            (返回Connecting)
```

### 状态转换条件

| 当前状态 | 事件 | 新状态 | 动作 |
|---------|------|-------|------|
| Disconnected | connectToPeer() | Connecting | QTcpSocket::connectToHost() |
| Connecting | connected() | Handshaking | sendHandshakeRequest() |
| Connecting | error() | Error | emit connectionFailed() |
| Handshaking | handshakeCompleted() | Connected | emit connectionEstablished() |
| Handshaking | timeout() | Error | emit connectionFailed() |
| Connected | disconnected() | Disconnected | 触发重连（如果启用） |
| Error | retry() | Connecting | 重新连接 |

---

## 🧪 测试设计

### 单元测试

#### TcpConnection握手测试
```cpp
TEST_F(TcpConnectionTest, HandshakeSuccess) {
    // Arrange
    TcpConnection conn("peer1", mockSocket);
    QSignalSpy handshakeSpy(&conn, &TcpConnection::handshakeCompleted);
    
    // Act
    conn.sendHandshakeRequest();
    
    // 模拟收到响应
    protocol::HandshakeResponse response;
    response.set_accepted(true);
    response.set_user_id("peer1_uuid");
    // ...发送响应到conn
    
    // Assert
    EXPECT_EQ(handshakeSpy.count(), 1);
}

TEST_F(TcpConnectionTest, HandshakeTimeout) {
    // Arrange
    TcpConnection conn("peer1", mockSocket);
    QSignalSpy failSpy(&conn, &TcpConnection::handshakeFailed);
    
    // Act
    conn.sendHandshakeRequest();
    QTest::qWait(6000); // 等待超时
    
    // Assert
    EXPECT_EQ(failSpy.count(), 1);
}
```

#### MessageService真实发送测试
```cpp
TEST_F(MessageServiceTest, SendToRealPeer) {
    // Arrange
    MessageService service;
    TcpConnectionManager* connMgr = mockConnectionManager();
    
    // Act
    service.sendTextMessage("peer1", "Hello");
    
    // Assert
    EXPECT_TRUE(connMgr->wasSendCalled("peer1"));
    EXPECT_NE(peerId, "echo_bot_local");
}
```

### 集成测试

#### 端到端通信测试
```cpp
TEST_F(P2PCommunicationIntegrationTest, TwoNodesExchangeMessages) {
    // Arrange
    TestApp app1(45678); // UDP端口
    TestApp app2(45679);
    
    // Act
    app1.start();
    app2.start();
    QTest::qWait(3000); // 等待节点发现和连接
    
    app1.sendMessage("app2_id", "Hello from App1");
    QTest::qWait(500);
    
    // Assert
    EXPECT_TRUE(app2.hasReceivedMessage("Hello from App1"));
    
    // 反向测试
    app2.sendMessage("app1_id", "Hello from App2");
    QTest::qWait(500);
    EXPECT_TRUE(app1.hasReceivedMessage("Hello from App2"));
}
```

---

## ⚡ 性能优化

### 握手优化
1. **并发握手**: 多节点同时握手，不阻塞
2. **快速失败**: 2秒超时，立即重试
3. **缓存验证**: UUID验证结果缓存，避免重复计算

### 消息发送优化
1. **批量发送**: 相同节点的消息合并发送
2. **优先级队列**: MessageQueue已实现
3. **压缩**: 大消息启用压缩（未来优化）

---

## 🔒 安全考虑

### UUID验证
```cpp
bool validatePeerUUID(const QString& receivedUUID, const QString& expectedUUID) {
    // 1. 检查UUID格式
    if (!QUuid(receivedUUID).isNull() == false) {
        return false;
    }
    
    // 2. 检查是否匹配（如果已知）
    if (!expectedUUID.isEmpty() && receivedUUID != expectedUUID) {
        qWarning() << "UUID mismatch: expected" << expectedUUID 
                   << "got" << receivedUUID;
        return false;
    }
    
    return true;
}
```

### 防止重放攻击
```cpp
// 握手时间戳验证
bool isHandshakeTimestampValid(int64 timestamp) {
    int64 now = QDateTime::currentMSecsSinceEpoch();
    int64 diff = qAbs(now - timestamp);
    
    // 允许5分钟时间差
    return diff < 5 * 60 * 1000;
}
```

---

## 📊 性能指标

### 基准测试目标

| 指标 | Windows | RK3566 |
|------|---------|--------|
| 连接建立 | <2s | <3s |
| 握手延迟 | <100ms | <200ms |
| 消息延迟 | <50ms | <100ms |
| 吞吐量 | >1000msg/s | >500msg/s |

### 监控点
```cpp
// 在关键路径添加性能日志
QElapsedTimer timer;
timer.start();

// ...操作...

qInfo() << "[Performance] Connection established in" << timer.elapsed() << "ms";
```

---

## 🔧 配置项

### 连接参数
```cpp
// TcpConnectionManager.h
struct ConnectionConfig {
    int handshakeTimeout = 5000;      // 握手超时（毫秒）
    int connectTimeout = 10000;       // 连接超时（毫秒）
    int retryInterval = 3000;         // 重试间隔（毫秒）
    int maxRetries = 3;               // 最大重试次数
    bool autoReconnect = true;        // 自动重连
};
```

---

## 🚀 部署计划

### Phase 1: 握手机制 (Day 1-2)
- [ ] Protobuf消息定义
- [ ] TcpConnection握手逻辑
- [ ] 单元测试

### Phase 2: 自动连接 (Day 3)
- [ ] PeerDiscovery信号连接
- [ ] TcpConnectionManager::onPeerDiscovered()
- [ ] 集成测试

### Phase 3: 消息路由 (Day 4)
- [ ] MessageService真实发送
- [ ] Echo Bot移除
- [ ] 端到端测试

### Phase 4: 测试与优化 (Day 5)
- [ ] 双平台测试
- [ ] 性能基准测试
- [ ] Bug修复

---

## 📚 参考资料

**Qt文档**:
- [QTcpSocket](https://doc.qt.io/qt-6/qtcpsocket.html)
- [QTcpServer](https://doc.qt.io/qt-6/qtcpserver.html)

**Protobuf文档**:
- [Protocol Buffers v3](https://protobuf.dev/)

**相关技术规格**:
- [TechSpec-002 TCP连接管理](TechSpec-002_tcp-connection-manager.md)
- [TechSpec-004 文本消息](TechSpec-004_text-messaging.md)

---

**创建时间**: 2024-11-20 01:10  
**最后更新**: 2024-11-20 01:10  
**版本**: 1.0  
**审核状态**: 待审核
