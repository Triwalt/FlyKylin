# US-004: 文本消息与聊天窗口

**创建日期**: 2024-11-19  
**状态**: 待开始  
**优先级**: Critical  
**故事点**: 6  
**Sprint**: Sprint 2  

---

## User Story

**作为** 企业员工  
**我希望** 通过独立聊天窗口与同事进行即时文本交流  
**以便** 快速沟通工作事项，提高协作效率

---

## 需求澄清记录

### 原始需求
实现类似QQ/微信的聊天功能，支持发送和接收文本消息，有完善的通知提醒。

### 澄清后的需求

1. **聊天窗口**: 独立的ChatWindow，双击在线用户打开
2. **消息通知**: 
   - 系统托盘图标闪烁
   - Windows Toast通知（原生API）
   - 提示音（可选）
3. **消息状态**: 
   - 发送中（⏳）
   - 已送达（✓）
   - 发送失败（❌）
4. **离线处理**: 对方离线时提示"对方不在线"，不缓存消息
5. **消息限制**: 单条最大10KB（约5000汉字）
6. **消息可靠性**: 简单ACK确认机制

### 技术可行性

- ✅ Qt QWidget实现独立聊天窗口
- ✅ QSystemTrayIcon实现托盘图标
- ✅ Windows Toast通知（Win10+ API）
- ✅ Protobuf TcpMessage传输文本
- ✅ 消息ACK机制保证可靠性

### 潜在风险

- ⚠️ 多窗口管理复杂度（缓解：ChatWindowManager单例）
- ⚠️ 消息乱序（缓解：序列号排序）
- ⚠️ 通知权限被禁用（缓解：降级到托盘闪烁）

---

## 验收标准

### AC1: 打开聊天窗口

- **Given** 在线用户列表显示一个在线用户
- **When** 双击该用户
- **Then** 打开独立聊天窗口，显示用户名和连接状态

### AC2: 发送文本消息

- **Given** 已打开聊天窗口且连接已建立
- **When** 输入文本"Hello"并点击发送
- **Then** 消息显示在聊天记录中，状态为"发送中⏳"，TCP发送成功后变为"已送达✓"

### AC3: 接收文本消息

- **Given** 已打开聊天窗口
- **When** 对方发送消息"Hi"
- **Then** 消息显示在聊天记录中，包含发送时间和内容

### AC4: 消息ACK确认

- **Given** 发送一条消息
- **When** 对方成功接收并回复ACK
- **Then** 消息状态更新为"已送达✓"

### AC5: 发送失败处理

- **Given** 发送消息时连接断开
- **When** 发送超时（5秒无响应）
- **Then** 消息状态更新为"发送失败❌"，显示重试按钮

### AC6: 离线提示

- **Given** 对方离线
- **When** 尝试发送消息
- **Then** 提示"对方不在线，无法发送消息"

### AC7: 系统托盘通知

- **Given** 聊天窗口未激活或最小化
- **When** 收到新消息
- **Then** 托盘图标闪烁3次，显示气泡通知"[用户名]: [消息内容]"

### AC8: Windows Toast通知

- **Given** Windows 10及以上系统
- **When** 收到新消息且窗口未激活
- **Then** 显示系统Toast通知，点击后激活聊天窗口

### AC9: 消息长度限制

- **Given** 输入超过5000汉字的文本
- **When** 点击发送
- **Then** 提示"消息过长，最多5000字"，禁止发送

### AC10: 多窗口管理

- **Given** 已打开3个聊天窗口
- **When** 双击第4个用户
- **Then** 打开新窗口，所有窗口独立显示

---

## 技术约束

### 平台

- Windows x64（支持Toast通知）
- RK3566 Linux（降级为托盘通知）

### 性能指标

- **消息发送延迟**: <100ms（已建立连接）
- **消息接收延迟**: <50ms（TCP接收后立即显示）
- **窗口打开时间**: <200ms
- **内存占用**: 单窗口<5MB，10窗口<50MB

### 依赖

- ✅ US-002（TCP连接管理器）
- ✅ Protobuf（TextMessage、MessageAck）
- ✅ Qt6::Widgets（QWidget、QTextEdit、QLineEdit）

### 接口设计

- `ChatWindow`: 独立聊天窗口
- `ChatWindowManager`: 窗口管理器（单例）
- `SystemNotifier`: 系统通知管理
- `MessageStatusTracker`: 消息状态追踪

---

## 非功能需求

### 可用性

- 聊天窗口支持复制粘贴
- 发送快捷键：Enter发送，Shift+Enter换行
- 消息时间戳显示（HH:mm:ss格式）

### 可靠性

- 消息ACK确认
- 发送失败自动重试（最多3次）
- 连接断开时禁用发送按钮

### 可维护性

- 窗口与业务逻辑分离（MVVM模式）
- 统一的消息格式（Protobuf）
- 清晰的状态管理

---

## 依赖关系

### 前置依赖

- ✅ US-002（TCP连接管理器）必须完成

### 后置依赖

- 无（本Sprint最后交付）

---

## 测试策略

### 单元测试

- `ChatWindow`: 消息显示、输入验证
- `ChatWindowManager`: 窗口创建、激活、关闭
- `SystemNotifier`: 通知触发
- `MessageStatusTracker`: 状态跟踪

### 集成测试

- 双节点对发消息，验证完整流程
- 消息ACK机制验证
- 离线消息处理验证
- 多窗口并发测试

### UI测试

- 聊天窗口布局正确性
- 消息状态图标显示
- 托盘通知交互

---

## 开发任务分解

### Task 1: ChatWindow基础UI (1.5 SP)

- 创建ChatWindow类和UI布局
- 实现消息显示区（QTextEdit）
- 实现输入框和发送按钮
- 实现连接状态栏

### Task 2: 消息发送和接收 (1.5 SP)

- 集成TcpConnectionManager
- 实现消息序列化（Protobuf）
- 实现消息发送逻辑
- 实现消息接收和显示

### Task 3: 消息状态和ACK (1 SP)

- 实现MessageStatusTracker
- 实现ACK发送和接收
- 实现消息状态更新UI

### Task 4: ChatWindowManager和通知 (1.5 SP)

- 实现ChatWindowManager单例
- 实现窗口管理（创建、激活、关闭）
- 实现SystemNotifier
- 实现托盘图标和Toast通知

### Task 5: 错误处理和优化 (0.5 SP)

- 实现离线提示
- 实现消息长度限制
- 实现发送失败重试
- 性能优化

---

## 里程碑

- **M1**: ChatWindow UI完成（Day 1-2）
- **M2**: 消息收发功能完成（Day 3-4）
- **M3**: ACK和状态管理完成（Day 5）
- **M4**: 通知功能完成（Day 6-7）
- **M5**: 集成测试通过（Day 8）

---

## 参考文档

- [Qt QSystemTrayIcon](https://doc.qt.io/qt-6/qsystemtrayicon.html)
- [Windows Toast通知API](https://docs.microsoft.com/en-us/windows/apps/design/shell/tiles-and-notifications/send-local-toast)
- [Protobuf messages.proto](../../protocol/messages.proto)
- [US-002: TCP连接管理器](./US-002_tcp-connection-manager.md)

---

**负责人**: Development Execution Agent  
**审核人**: Architecture Design Agent  
**最后更新**: 2024-11-19
