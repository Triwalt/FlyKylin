# US-002: TCP长连接管理器

**创建日期**: 2024-11-19  
**状态**: 待开始  
**优先级**: Critical  
**故事点**: 4  
**Sprint**: Sprint 2  

---

## User Story

**作为** 企业员工  
**我希望** 与局域网内的同事建立稳定的TCP连接  
**以便** 能够即时发送和接收消息，无需等待连接建立

---

## 需求澄清记录

### 原始需求
实现TCP长连接管理，支持点对点通信，确保消息能够可靠、快速地传输。

### 澄清后的需求
1. **连接策略**: 智能长连接池，活跃用户保持连接，闲置5分钟自动断开
2. **连接上限**: 最多同时保持20个TCP连接（适配RK3566性能）
3. **重试策略**: 5次指数退避重试，间隔1s/2s/4s/8s/16s，最大30秒
4. **状态管理**: 使用状态机模式，清晰的状态转换和GUI反馈
5. **心跳保活**: 30秒发送一次心跳，60秒无响应判定离线
6. **消息队列**: 优先级队列（Critical > High > Normal > Low）

### 技术可行性
- ✅ Qt6的`QTcpSocket`和`QTcpServer`成熟稳定
- ✅ 状态机模式适合连接管理
- ✅ 20个连接在RK3566上内存占用<100KB，CPU开销<1%
- ✅ 局域网TCP连接建立时间<50ms

### 潜在风险
- ⚠️ 网络抖动导致频繁重连（缓解：指数退避+抖动）
- ⚠️ 连接数过多导致资源耗尽（缓解：20个硬上限+闲置清理）
- ⚠️ 防火墙阻断TCP连接（缓解：明确错误提示）

---

## 验收标准

### AC1: 连接建立
- **Given** 发现一个在线节点
- **When** 用户打开与该节点的聊天窗口
- **Then** 自动建立TCP连接，GUI显示"正在连接..."，10-50ms内完成

### AC2: 连接重试
- **Given** TCP连接建立失败
- **When** 系统自动重试
- **Then** 最多重试5次，间隔指数增长（1s/2s/4s/8s/16s），GUI显示"正在重试（第X次）..."

### AC3: 连接保活
- **Given** 已建立的TCP连接
- **When** 5分钟内无消息收发
- **Then** 自动断开连接，释放资源

### AC4: 心跳检测
- **Given** 活跃的TCP连接
- **When** 30秒无消息收发
- **Then** 发送心跳包，60秒无响应则判定对方离线并断开连接

### AC5: 连接上限
- **Given** 已有20个活跃TCP连接
- **When** 尝试建立第21个连接
- **Then** 拒绝新连接，提示"连接数已达上限，请稍后重试"

### AC6: 状态通知
- **Given** TCP连接状态变化
- **When** 状态从"连接中"变为"已连接"
- **Then** 发出`stateChanged`信号，GUI更新连接状态显示

### AC7: 消息队列
- **Given** 连接正在重试中
- **When** 用户发送消息
- **Then** 消息进入优先级队列，连接成功后自动发送

### AC8: 错误处理
- **Given** TCP连接异常断开
- **When** 检测到连接断开
- **Then** 清理资源，发出`disconnected`信号，GUI提示"连接已断开"

---

## 技术约束

### 平台
- Windows x64（Visual Studio 2022编译）
- RK3566 Linux（GCC 11+）

### 性能指标
- **连接建立时间**: 局域网<50ms
- **心跳间隔**: 30秒
- **超时判定**: 60秒无响应
- **闲置超时**: 5分钟无活动
- **内存占用**: 单连接<5KB，20连接<100KB
- **CPU占用**: 心跳检测<1%

### 依赖
- ✅ Qt6::Network（QTcpSocket、QTcpServer）
- ✅ Protobuf（TcpMessage、MessageAck）
- ✅ PeerDiscovery（节点发现，获取IP和端口）

### 接口设计
- `TcpConnectionManager`: 连接池管理（单例）
- `TcpConnection`: 单个连接封装（状态机）
- `MessageQueue`: 优先级消息队列
- `RetryStrategy`: 智能重试策略

---

## 非功能需求

### 可靠性
- 连接失败自动重试，最多5次
- 消息队列缓存，连接成功后自动发送
- 异常断开自动清理资源

### 可维护性
- 清晰的状态机模式
- 统一的错误处理机制
- 详细的日志记录

### 可扩展性
- 预留加密接口（`I_MessageEncryption`）
- 消息类型可扩展（Protobuf `MessageType`）
- 队列优先级可配置

---

## 依赖关系

### 前置依赖
- ✅ Protobuf集成完成（Tech-002）
- ✅ PeerDiscovery UDP广播（Tech-001）

### 后置依赖
- US-004（文本消息）依赖本Story

---

## 测试策略

### 单元测试
- `TcpConnectionManager`: 连接池管理、上限控制
- `TcpConnection`: 状态转换、心跳、重试逻辑
- `MessageQueue`: 优先级排序、去重
- `RetryStrategy`: 指数退避算法

### 集成测试
- 模拟网络断开，验证重连机制
- 模拟连接超时，验证重试策略
- 模拟消息队列积压，验证优先级发送
- 双节点对发消息，验证并发处理

### 性能测试
- 20个并发连接的内存和CPU占用
- 1000条消息的队列吞吐量
- 网络抖动下的重连速度

---

## 开发任务分解

### Task 1: TcpConnection基础类 (1 SP)
- 实现状态机（Disconnected/Connecting/Connected/Reconnecting/Failed）
- 实现连接建立和断开
- 实现数据发送和接收

### Task 2: 重试和心跳机制 (1 SP)
- 实现指数退避重试策略
- 实现心跳保活机制
- 实现超时检测

### Task 3: TcpConnectionManager连接池 (1 SP)
- 实现连接池管理（QMap存储）
- 实现20个连接上限控制
- 实现闲置连接清理

### Task 4: 优先级消息队列 (1 SP)
- 实现优先级队列（QMultiMap）
- 实现失败消息重试队列
- 实现队列去重

---

## 里程碑

- **M1**: TcpConnection基础功能完成（Day 1-2）
- **M2**: 重试和心跳机制完成（Day 3-4）
- **M3**: 连接池和队列完成（Day 5-6）
- **M4**: 单元测试通过（Day 7）
- **M5**: 集成测试通过（Day 8）

---

## 参考文档

- [Qt QTcpSocket文档](https://doc.qt.io/qt-6/qtcpsocket.html)
- [State Machine Pattern](https://refactoring.guru/design-patterns/state)
- [Exponential Backoff算法](https://en.wikipedia.org/wiki/Exponential_backoff)
- [Protobuf messages.proto](../../protocol/messages.proto)

---

**负责人**: Development Execution Agent  
**审核人**: Architecture Design Agent  
**最后更新**: 2024-11-19
