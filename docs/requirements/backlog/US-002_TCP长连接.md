# User Story: US-002 TCP长连接消息传输

**标题**: US-002 建立TCP长连接进行可靠消息传输

**优先级**: Critical  
**故事点**: 8  
**Sprint**: Sprint 1  
**状态**: 待开发

---

## User Story

**作为** 企业局域网用户  
**我希望** 与发现的在线用户建立稳定的TCP连接  
**以便** 可靠地收发文本消息

## 验收标准

### AC1: 建立TCP连接
- **Given** 用户A通过UDP发现了用户B
- **When** 用户A首次向用户B发送消息
- **Then** 应用自动建立TCP连接到用户B的监听端口
- **And** 连接成功后保持长连接状态

### AC2: 发送文本消息
- **Given** 用户A和用户B已建立TCP连接
- **When** 用户A发送文本"Hello World"
- **Then** 消息通过Protobuf序列化
- **And** 通过TCP连接发送给用户B
- **And** 用户B接收并反序列化消息
- **And** 在聊天窗口显示"用户A: Hello World"

### AC3: 接收文本消息
- **Given** 用户A和用户B已建立TCP连接
- **When** 用户B发送消息
- **Then** 用户A的TCP Socket接收到数据
- **And** 应用解析Protobuf消息
- **And** 触发`messageReceived`信号
- **And** UI更新显示新消息

### AC4: 心跳保活
- **Given** TCP连接已建立
- **When** 60秒内无业务消息
- **Then** 自动发送PING心跳消息
- **And** 对方回复PONG
- **And** 连接保持活跃状态

### AC5: 断线重连
- **Given** TCP连接异常断开
- **When** 应用检测到连接断开
- **Then** 等待5秒后自动重连
- **And** 重连成功后恢复消息收发
- **And** 重连失败3次后标记节点为离线

## 技术约束

### 平台支持
- Windows x64 (MSVC 2022)

### 性能目标
- 消息发送延迟 < 100ms
- 单TCP连接吞吐 > 10MB/s
- 支持100+并发TCP连接

### 网络协议
- **明文传输**: 不使用TLS/SSL
- **消息分隔**: 使用长度前缀（4字节Length + Protobuf Payload）
- **端口**: TCP 45679 (可配置)

### 依赖
- US-001 UDP节点发现（必须先完成）
- Qt Network模块
- Protobuf序列化

## 技术设计

### 涉及模块
- `src/core/communication/PeerSession.h/cpp`
- `src/core/communication/CommunicationService.h/cpp`
- `src/protobuf/message.proto`

### Protobuf消息扩展
```protobuf
enum MessageType {
    MSG_ONLINE = 0;
    MSG_OFFLINE = 1;
    MSG_HEARTBEAT = 2;
    MSG_PING = 3;        // 心跳PING
    MSG_PONG = 4;        // 心跳PONG
    MSG_TEXT = 10;       // 文本消息
}

message TextMessage {
    string content = 1;
    int64 timestamp = 2;
}

message Message {
    MessageType type = 1;
    UserInfo sender = 2;
    oneof payload {
        TextMessage text = 10;
    }
}
```

### 核心类设计
```cpp
// PeerSession: 管理单个TCP连接
class PeerSession : public QObject {
    Q_OBJECT
public:
    explicit PeerSession(const PeerNode& peer, QObject* parent = nullptr);
    bool connectToPeer();
    void sendTextMessage(const QString& text);
    void sendHeartbeat();
    
signals:
    void connected();
    void disconnected();
    void messageReceived(const Message& msg);
    void errorOccurred(const QString& error);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onHeartbeatTimer();
    
private:
    QTcpSocket* m_socket;
    QTimer* m_heartbeatTimer;
    PeerNode m_peer;
    QByteArray m_receiveBuffer;
    
    void processReceivedData();
    void sendMessage(const Message& msg);
};

// CommunicationService: 管理所有连接
class CommunicationService : public QObject {
    Q_OBJECT
public:
    static CommunicationService* instance();
    
    void startServer(quint16 tcpPort = 45679);
    void sendTextTo(const QString& userId, const QString& text);
    
signals:
    void peerConnected(const QString& userId);
    void peerDisconnected(const QString& userId);
    void textMessageReceived(const QString& userId, const QString& text);
    
private:
    QTcpServer* m_server;
    QMap<QString, PeerSession*> m_sessions; // userId -> session
    
    PeerSession* getOrCreateSession(const QString& userId);
};
```

### 消息格式（Wire Protocol）
```
+----------------+------------------+
| Length (4字节) | Protobuf Payload |
+----------------+------------------+
```
- Length: uint32大端序，表示Payload字节数
- Payload: Protobuf序列化的Message

## 测试策略

### 单元测试
- Protobuf序列化/反序列化
- 消息长度前缀编解码
- 心跳超时逻辑

### 集成测试
- 2个实例建立连接并互发消息
- 模拟连接断开和重连
- 大量消息发送（1000条）性能测试

### 压力测试
- 10个节点同时连接
- 每秒100条消息吞吐
- 长时间运行稳定性（24小时）

## 风险与缓解

### 风险1: TCP粘包问题
- **缓解**: 使用长度前缀协议，严格解析
- **测试**: 压力测试大量小消息

### 风险2: 连接泄漏
- **缓解**: 使用智能指针管理Session
- **测试**: 内存泄漏检测工具

### 风险3: 消息乱序
- **缓解**: TCP保证顺序，无需额外处理
- **验证**: 集成测试验证消息顺序

## Definition of Done

- [ ] `PeerSession`和`CommunicationService`实现完成
- [ ] Protobuf消息定义完善
- [ ] 单元测试覆盖率 ≥ 70%
- [ ] 2个实例互发100条消息测试通过
- [ ] 断线重连测试通过
- [ ] 性能目标达成（<100ms延迟）
- [ ] Code Review通过
- [ ] 无内存泄漏（Valgrind验证）

## 相关Story
- US-001 UDP节点发现（前置依赖）
- US-003 基础UI用户列表（并行开发）
- US-004 聊天窗口UI（Sprint 2）
