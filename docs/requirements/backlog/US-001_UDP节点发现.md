# User Story: US-001 UDP节点发现

**标题**: US-001 局域网P2P节点自动发现

**优先级**: Critical  
**故事点**: 5  
**Sprint**: Sprint 1  
**状态**: 待开发

---

## User Story

**作为** 企业局域网用户  
**我希望** 应用启动后自动发现局域网内其他在线用户  
**以便** 无需手动配置即可建立P2P通信

## 验收标准

### AC1: UDP广播节点上线
- **Given** 用户A启动FlyKylin应用
- **When** 应用完成初始化
- **Then** 应用每5秒通过UDP广播发送"上线通知"消息
- **And** 消息包含：IP地址、用户名、主机名、端口号

### AC2: 接收并识别其他节点
- **Given** 用户B已在线
- **When** 用户A启动应用并发送广播
- **Then** 用户B接收到用户A的上线通知
- **And** 用户B将用户A添加到"在线用户列表"
- **And** 在UI上显示用户A的信息

### AC3: 节点离线检测
- **Given** 用户A和用户B已建立连接
- **When** 用户A关闭应用或网络断开
- **Then** 用户B在30秒内未收到用户A的心跳
- **And** 用户B将用户A标记为离线并从在线列表移除

### AC4: 多用户发现
- **Given** 局域网内有5个在线用户
- **When** 用户F启动应用
- **Then** 用户F能发现所有5个在线用户
- **And** 在线用户列表显示5个节点信息

## 技术约束

### 平台支持
- Windows x64 (MSVC 2022)
- RK3566 Linux ARM64 (延后到Sprint 4)

### 性能目标
- P2P节点发现延迟 < 5秒
- UDP广播间隔: 5秒
- 离线检测时间: 30秒

### 网络协议
- **不使用加密**: UDP广播为明文
- **不使用认证**: IP+主机名作为身份标识
- **端口**: UDP 45678 (可配置)

### 依赖
- Qt Network模块
- Protobuf消息定义

## 技术设计

### 涉及模块
- `src/core/communication/PeerDiscovery.h/cpp`
- `src/core/models/PeerNode.h`
- `src/protobuf/message.proto`

### Protobuf消息定义
```protobuf
message UserInfo {
    string user_id = 1;      // IP地址
    string user_name = 2;    // 用户名
    string host_name = 3;    // 主机名
    int32 tcp_port = 4;      // TCP监听端口
    int64 timestamp = 5;     // 时间戳
}

message Message {
    MessageType type = 1;    // MSG_ONLINE/MSG_OFFLINE/MSG_HEARTBEAT
    UserInfo user_info = 2;
}
```

### 核心类设计
```cpp
class PeerDiscovery : public QObject {
    Q_OBJECT
public:
    explicit PeerDiscovery(QObject* parent = nullptr);
    void start(quint16 udpPort = 45678);
    void stop();
    
signals:
    void peerFound(const PeerNode& node);
    void peerOffline(const QString& userId);
    
private slots:
    void onBroadcastTimer();
    void onDatagramReceived();
    
private:
    QUdpSocket* m_socket;
    QTimer* m_broadcastTimer;
    QMap<QString, QDateTime> m_lastSeen; // userId -> 最后心跳时间
};
```

## 测试策略

### 单元测试
- UDP消息序列化/反序列化
- PeerNode对象创建和比较
- 心跳超时检测逻辑

### 集成测试
- 启动2个实例，验证互相发现
- 模拟网络断开，验证离线检测
- 压力测试：10+节点同时在线

### 性能测试
- 测量节点发现延迟
- 测量UDP广播CPU占用
- 测量内存占用（节点列表）

## 风险与缓解

### 风险1: UDP广播被防火墙拦截
- **缓解**: 文档说明需要允许UDP 45678端口
- **备选**: 提供手动添加IP功能

### 风险2: 跨网段无法发现
- **缓解**: 文档说明仅支持同一子网
- **备选**: 考虑mDNS（Sprint 2+）

### 风险3: IP地址变化导致身份混淆
- **缓解**: 使用MAC地址或生成UUID作为user_id
- **优先级**: Medium（Sprint 2优化）

## Definition of Done

- [ ] `PeerDiscovery`类实现完成
- [ ] Protobuf消息定义并生成C++代码
- [ ] 单元测试覆盖率 ≥ 80%
- [ ] 2个Windows实例互相发现测试通过
- [ ] 性能目标达成（<5秒发现）
- [ ] Code Review通过
- [ ] 文档更新（API文档）

## 相关Story
- US-002 TCP长连接建立（依赖本Story）
- US-003 基础UI用户列表（依赖本Story）
