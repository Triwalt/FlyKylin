# US-005: 真实P2P消息通信

**状态**: 📋 待开始  
**优先级**: P0 - Critical  
**故事点**: 8  
**Sprint**: Sprint 3  
**创建日期**: 2024-11-20  
**负责人**: Development Execution Agent

---

## 📖 User Story

**作为** 飞秋用户  
**我希望** 能与局域网内其他用户建立真实的P2P连接并收发消息  
**以便** 在没有中心服务器的情况下进行点对点通信

---

## ✅ 验收标准 (Acceptance Criteria)

### AC1: TCP连接自动建立
**Given** 两个实例在同一局域网内运行  
**When** PeerDiscovery发现新节点  
**Then** 自动建立TCP连接，连接状态变为"已连接"

### AC2: 消息双向收发
**Given** 两个节点已建立TCP连接  
**When** 用户A发送消息"Hello"给用户B  
**Then** 用户B能在聊天窗口中收到"Hello"消息

### AC3: 消息顺序保证
**Given** 用户A连续发送3条消息: "1", "2", "3"  
**When** 用户B接收消息  
**Then** 消息按顺序显示: "1", "2", "3"

### AC4: 连接断开重连
**Given** 两个节点已建立连接  
**When** 网络临时中断（拔网线5秒）  
**Then** 网络恢复后自动重连，消息继续收发

### AC5: 多节点同时通信
**Given** 局域网内有3个节点: A, B, C  
**When** 节点A同时与B、C建立连接  
**Then** 可同时向B和C发送消息，互不干扰

### AC6: Echo Bot替换
**Given** US-005实现完成  
**When** 启动应用程序  
**Then** 不再显示Echo Bot虚拟节点，只显示真实发现的节点

### AC7: 双平台验证
**Given** Windows和RK3566设备在同一局域网  
**When** 相互发送消息  
**Then** 消息正常收发，延迟<500ms（Windows）/<1s（RK3566）

---

## 🎯 业务价值

### 核心价值
1. **真实可用性**: 从Demo进化到真正可用的通信工具
2. **去中心化**: 无需中心服务器，完全P2P架构
3. **低延迟**: 直连通信，延迟更低

### 用户影响
- **企业用户**: 可在内网无服务器环境下使用
- **开发团队**: 验证P2P架构可行性
- **项目演示**: 毕业答辩时展示真实通信能力

### 技术价值
- 建立完整的TCP连接管理机制
- 验证Protobuf序列化在真实网络的表现
- 为后续功能（文件传输、音视频）打下基础

---

## 🔧 技术约束

### 平台支持
- **Windows x64**: 使用Qt6 QTcpSocket
- **RK3566 Linux ARM64**: 使用Qt6 QTcpSocket

### 性能要求
| 指标 | Windows | RK3566 |
|------|---------|--------|
| 连接建立时间 | <2秒 | <3秒 |
| 消息发送延迟 | <100ms | <200ms |
| 同时连接数 | ≥10 | ≥5 |
| 内存占用 | <50MB | <30MB |

### 依赖模块
- ✅ PeerDiscovery（UDP节点发现）- Sprint 1已完成
- ✅ TcpConnectionManager - Sprint 2已完成
- ✅ MessageService - Sprint 2已完成
- ✅ Protobuf协议 - Sprint 2已完成
- 🆕 UserProfile单例 - Sprint 3 TD-009

### 技术栈
- C++20
- Qt 6.5+ (QTcpSocket, QTcpServer)
- Protobuf 3.x
- GoogleTest (测试)

---

## 📋 任务分解

### Phase 1: 设计 (6h)
1. **Task 3.1.1**: TCP握手协议设计 (2h)
   - 定义握手Protobuf消息
   - 设计握手流程（SYN/SYN-ACK/ACK）
   - 设计认证机制（UUID验证）

2. **Task 3.1.2**: 连接状态机设计 (2h)
   - 状态定义：Disconnected → Connecting → Connected → Error
   - 状态转换条件
   - 超时和重试策略

3. **Task 3.1.3**: 架构集成设计 (2h)
   - PeerDiscovery → TcpConnectionManager集成点
   - MessageService真实发送流程
   - Echo Bot移除策略

### Phase 2: 开发 (11h)
4. **Task 3.1.4**: 实现TCP握手逻辑 (4h)
   - 文件: `TcpConnectionManager.cpp`
   - 实现握手消息发送/接收
   - 实现UUID验证
   - 实现握手超时处理

5. **Task 3.1.5**: 集成PeerDiscovery (3h)
   - 文件: `PeerDiscovery.cpp`, `TcpConnectionManager.cpp`
   - onPeerDiscovered信号连接到TcpConnectionManager
   - 自动建立TCP连接
   - 连接失败重试

6. **Task 3.1.6**: MessageService真实发送 (3h)
   - 文件: `MessageService.cpp`
   - 移除Echo Bot路由逻辑
   - 使用真实TCP连接发送
   - 错误处理和状态更新

7. **Task 3.1.7**: MainWindow移除Echo Bot (1h)
   - 文件: `MainWindow.cpp`
   - 删除Echo Bot节点添加代码
   - 更新UI逻辑

### Phase 3: 测试 (4h)
8. **Task 3.1.8**: 集成测试编写 (4h)
   - 文件: `tests/integration/P2PCommunication_test.cpp`
   - 测试：两节点握手
   - 测试：消息收发
   - 测试：断线重连
   - 测试：多节点并发

9. **Task 3.1.9**: 双平台手动测试 (2h)
   - Windows → Windows
   - Windows → RK3566
   - 性能基准测试

**总计工时**: 21小时

---

## 🔬 测试策略

### 单元测试
- `TcpConnectionManager::handleHandshake()` - 握手逻辑
- `MessageService::sendRealMessage()` - 真实发送
- 覆盖率目标: ≥80%

### 集成测试
- 两节点握手流程
- 消息收发端到端
- 断线重连
- 多节点并发

### 手动测试场景
1. **基本通信**
   - 步骤: 启动两个实例，互相发送消息
   - 预期: 消息正常收发

2. **断线重连**
   - 步骤: 发送消息中途断网，5秒后恢复
   - 预期: 自动重连，未发送的消息继续发送

3. **多节点**
   - 步骤: 启动3个实例，A同时与B、C通信
   - 预期: 消息互不干扰

4. **性能测试**
   - 步骤: 连续发送100条消息
   - 预期: Windows平均延迟<100ms

---

## ⚠️ 风险识别

### 风险1: 调试困难 🔴
**描述**: 需要两台设备或两个实例测试  
**影响**: High  
**概率**: High  
**缓解措施**:
- 优先单机两实例测试
- 添加详细日志（连接状态、消息流向）
- 准备Wireshark抓包工具
- 保留Echo Bot作为降级方案

### 风险2: NAT穿透 🟡
**描述**: 复杂网络环境可能无法直连  
**影响**: Medium  
**概率**: Low（局域网场景）  
**缓解措施**:
- Sprint 3仅支持局域网
- 文档说明限制
- Sprint 4考虑NAT穿透

### 风险3: 握手超时 🟡
**描述**: 网络抖动导致握手失败  
**影响**: Medium  
**概率**: Medium  
**缓解措施**:
- 实现重试机制（3次）
- 增加超时时间（5秒）
- 显示连接状态给用户

### 风险4: 消息乱序 🟢
**描述**: TCP虽然保证顺序，但并发处理可能乱序  
**影响**: Low  
**概率**: Low  
**缓解措施**:
- 使用消息序列号验证
- 单线程接收处理
- 测试验证顺序性

---

## 🔗 依赖关系

### 前置依赖
- ✅ US-001: UDP节点发现（Sprint 1）
- ✅ US-002: TCP连接管理（Sprint 2）
- ✅ US-004: 文本消息功能（Sprint 2）

### 并行依赖
- 🔄 TD-009: UserProfile单例（建议优先完成）

### 后续Story
- US-006: 消息持久化（Sprint 4）
- US-007: 文件传输（Sprint 5）

---

## 📊 DoD检查清单

- [ ] 所有7个AC通过验证
- [ ] 单元测试覆盖率≥80%
- [ ] 集成测试通过（4个场景）
- [ ] Windows x64手动测试通过
- [ ] RK3566 ARM64手动测试通过（或CI通过）
- [ ] 编译零警告
- [ ] Code Review通过
- [ ] 性能基准达标
- [ ] 文档更新：
  - [ ] API文档（TcpConnectionManager握手接口）
  - [ ] 用户文档（使用说明）
  - [ ] Changelog记录

---

## 💡 实现建议

### 握手协议设计
```protobuf
message HandshakeRequest {
    string protocol_version = 1;  // "1.0"
    string user_id = 2;           // UUID
    string user_name = 3;
    int64 timestamp = 4;
}

message HandshakeResponse {
    bool accepted = 1;
    string user_id = 2;
    string user_name = 3;
    string error_message = 4;
}
```

### 状态机实现
```cpp
enum class ConnectionState {
    Disconnected,
    Connecting,
    Handshaking,
    Connected,
    Error
};

void TcpConnectionManager::handleStateChange(
    const QString& peerId, 
    ConnectionState newState
) {
    // 状态转换逻辑
    // 发送信号通知UI
}
```

### 集成点
```cpp
// PeerDiscovery.cpp
void PeerDiscovery::processReceivedMessage(...) {
    // ...现有逻辑
    if (node.isOnline() && !m_peers.contains(userId)) {
        // 新节点发现，触发TCP连接
        emit peerDiscovered(node);  // 连接到TcpConnectionManager
    }
}

// TcpConnectionManager.cpp
void TcpConnectionManager::onPeerDiscovered(const PeerNode& node) {
    if (!m_connections.contains(node.userId())) {
        // 建立TCP连接
        connectToPeer(node.ipAddress(), node.tcpPort());
    }
}
```

---

## 📚 参考资料

**相关文档**:
- [TechSpec-002 TCP连接管理](TechSpec-002_tcp-connection-manager.md)
- [TechSpec-004 文本消息](TechSpec-004_text-messaging.md)
- [Sprint 3 Plan](../../sprints/active/sprint_3/plan.md)

**技术资源**:
- [Qt QTcpSocket文档](https://doc.qt.io/qt-6/qtcpsocket.html)
- [Protobuf Protocol](https://protobuf.dev/)

---

**创建时间**: 2024-11-20 01:05  
**最后更新**: 2024-11-20 01:05  
**版本**: 1.0
