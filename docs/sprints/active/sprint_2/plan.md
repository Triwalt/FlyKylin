# Sprint 2 计划 - 架构现代化基础设施

**Sprint周期**: 2024-11-19 ~ 2024-12-02 (2周)  
**主题**: "建立新架构基石，不影响现有功能"  
**负责人**: Sprint Planning Agent

---

## 📋 Sprint目标

### 核心目标

1. **✅ 构建系统现代化**: CMakePresets.json标准化构建流程
2. **✅ Protobuf协议集成**: 清理技术债务TD-001，实现结构化通信
3. **✅ 六边形架构基础**: 引入接口抽象层，提升可测试性
4. **✅ TCP长连接实现**: 完成Sprint 1推迟的US-002
5. **✅ 1v1文本聊天**: 实现基础聊天功能（US-004）

### 成功标准

- ✅ `cmake --preset windows-release` 一键构建
- ✅ 所有网络消息使用Protobuf序列化
- ✅ 接口抽象覆盖率≥80%（核心服务）
- ✅ TCP连接稳定性测试通过
- ✅ 双实例聊天功能验证

### 不包含范围

- ❌ QML UI迁移（推迟到Sprint 3）
- ❌ 协程重构（推迟到Sprint 4）
- ❌ Docker环境（推迟到Sprint 5）
- ❌ 文件传输功能（推迟到Sprint 3）

---

## 📊 Story Points分配

**Sprint容量**: 16 SP（基于Sprint 1 Velocity 14 + 经验提升）

| User Story | Story Points | 优先级 | 状态 |
|-----------|-------------|--------|------|
| US-002: TCP长连接 | 4 SP | P0 | ⏳ 待开始 |
| US-004: 文本消息收发 | 6 SP | P0 | ⏳ 待开始 |
| Tech-001: CMakePresets | 2 SP | P0 | ✅ 已完成 |
| Tech-002: Protobuf集成 | 3 SP | P0 | ⏳ 待开始 |
| Tech-003: 接口抽象层 | 2 SP | P1 | ✅ 已完成 |
| **总计** | **17 SP** | - | **12% 完成** |

**注**: 超出容量1 SP，但Tech-001和Tech-003已提前完成，实际剩余15 SP。

---

## 📝 User Stories详情

### US-002: TCP长连接 (4 SP) - P0

**状态**: ⏳ 待开始  
**负责人**: Development Execution Agent  
**依赖**: Tech-002 (Protobuf)

#### 描述

作为用户，我希望与其他节点建立稳定的TCP长连接，以便进行可靠的数据传输。

#### 验收标准

- [ ] AC1: 应用启动后监听TCP端口
- [ ] AC2: 接收其他节点的TCP连接请求
- [ ] AC3: 主动连接其他节点的TCP端口
- [ ] AC4: 连接断开后自动重连（最多3次）
- [ ] AC5: 心跳检测保持连接活跃（30秒间隔）

#### 技术实现

**接口设计**:
```cpp
namespace flykylin::domain {
    class TcpConnectionManager {
        I_NetworkAdapter& m_network;
        I_MessageSerializer& m_serializer;
    public:
        Task<int> connectToPeer(const PeerNode& peer);
        void disconnectPeer(const QString& userId);
        bool isConnected(const QString& userId) const;
    };
}
```

**测试计划**:
- 单元测试：Mock I_NetworkAdapter，测试连接逻辑
- 集成测试：双实例测试连接建立和断开
- 压力测试：10个并发连接稳定性

#### DoD (Definition of Done)

- [ ] 代码通过Code Review
- [ ] 单元测试覆盖率≥85%
- [ ] 集成测试全部通过
- [ ] 文档注释完整（Doxygen）
- [ ] 无内存泄漏（Valgrind验证）

---

### US-004: 文本消息收发 (6 SP) - P0

**状态**: ⏳ 待开始  
**负责人**: Development Execution Agent  
**依赖**: US-002 (TCP连接)

#### 描述

作为用户，我希望能够发送和接收文本消息，以便与其他用户进行1v1聊天。

#### 验收标准

- [ ] AC1: 发送文本消息到指定用户
- [ ] AC2: 接收其他用户的文本消息
- [ ] AC3: 消息显示在UI中（ViewModel更新）
- [ ] AC4: 消息持久化到数据库
- [ ] AC5: 离线消息缓存（对方不在线时）

#### 技术实现

**Protobuf定义**:
```protobuf
message TextMessage {
  string message_id = 1;
  string from_user_id = 2;
  string to_user_id = 3;
  string content = 4;
  uint64 timestamp = 5;
}
```

**服务设计**:
```cpp
namespace flykylin::domain {
    class MessageService {
        TcpConnectionManager& m_connectionMgr;
        I_MessageSerializer& m_serializer;
    public:
        Task<void> sendTextMessage(const QString& toUserId, const QString& content);
        Signal<void(const Message&)> onMessageReceived;
    };
}
```

**数据库Schema**:
```sql
CREATE TABLE messages (
    id TEXT PRIMARY KEY,
    from_user_id TEXT NOT NULL,
    to_user_id TEXT NOT NULL,
    content TEXT NOT NULL,
    timestamp INTEGER NOT NULL,
    is_read INTEGER DEFAULT 0
);
```

#### DoD

- [ ] 代码通过Code Review
- [ ] 单元测试覆盖率≥85%
- [ ] 集成测试：双实例聊天验证
- [ ] 数据库迁移脚本完成
- [ ] UI ViewModel正确更新

---

### Tech-001: CMakePresets标准化 (2 SP) - P0

**状态**: ✅ 已完成  
**负责人**: Architecture Design Agent  
**完成日期**: 2024-11-19

#### 描述

引入CMakePresets.json，标准化构建配置，消除环境变量依赖。

#### 成果

- ✅ 创建`CMakePresets.json`（windows/linux, release/debug）
- ✅ 集成vcpkg toolchain
- ✅ 定义buildPresets和testPresets
- ✅ 文档更新（README.md）

#### 影响

- **消除脚本**: configure-environment.ps1不再必需
- **构建命令简化**: 
  ```bash
  # 之前
  .\tools\developer\configure-environment.ps1
  cmake -B build -DVCPKG_BUILD_TYPE=release ...
  
  # 现在
  cmake --preset windows-release
  cmake --build build/windows-release
  ```

---

### Tech-002: Protobuf核心协议 (3 SP) - P0

**状态**: ⏳ 待开始  
**负责人**: Development Execution Agent  
**依赖**: 无

#### 描述

实现Protobuf序列化，替换文本协议，清理技术债务TD-001。

#### 任务分解

1. **定义.proto文件** (1 SP)
   - [ ] protocol/messages.proto（已创建框架）
   - [ ] 定义DiscoveryMessage
   - [ ] 定义TextMessage
   - [ ] 定义TcpMessage包装器

2. **CMake集成** (0.5 SP)
   - [ ] find_package(Protobuf)
   - [ ] protobuf_generate_cpp()
   - [ ] 链接生成的.pb.cc/.pb.h

3. **实现ProtobufSerializer** (1 SP)
   - [ ] 实现I_MessageSerializer接口
   - [ ] serializePeerAnnounce()
   - [ ] deserializePeerMessage()
   - [ ] serializeTextMessage()

4. **测试** (0.5 SP)
   - [ ] 单元测试：序列化/反序列化
   - [ ] 版本兼容性测试
   - [ ] 性能基准测试

#### DoD

- [ ] 所有网络消息使用Protobuf
- [ ] 测试覆盖率≥90%
- [ ] 技术债务TD-001标记为完成
- [ ] ADR-004记录Protobuf版本管理策略

---

### Tech-003: 接口抽象层 (2 SP) - P1

**状态**: ✅ 已完成  
**负责人**: Architecture Design Agent  
**完成日期**: 2024-11-19

#### 描述

引入六边形架构，定义核心接口抽象层，提升可测试性。

#### 成果

- ✅ 创建`I_NetworkAdapter.h`接口
- ✅ 创建`I_MessageSerializer.h`接口
- ✅ 定义NetworkAddress/NetworkMessage数据结构
- ✅ Doxygen文档完整

#### 下一步

- 🔜 实现QtNetworkAdapter（Sprint 2）
- 🔜 实现ProtobufSerializer（Sprint 2）
- 🔜 重构PeerDiscovery使用接口（Sprint 2）

---

## 🗓️ Sprint时间线

### Week 1 (2024-11-19 ~ 2024-11-25)

**Day 1-2**: Protobuf集成
- [ ] 完善protocol/messages.proto
- [ ] CMake集成protoc
- [ ] 生成C++代码验证

**Day 3-4**: ProtobufSerializer实现
- [ ] 实现I_MessageSerializer接口
- [ ] 单元测试（序列化/反序列化）
- [ ] 性能基准测试

**Day 5**: US-002开始（TCP连接）
- [ ] QtNetworkAdapter实现
- [ ] TcpConnectionManager设计

### Week 2 (2024-11-26 ~ 2024-12-02)

**Day 6-8**: US-002完成（TCP连接）
- [ ] TcpConnectionManager实现
- [ ] 集成测试（双实例）
- [ ] 重连机制和心跳

**Day 9-10**: US-004实现（文本消息）
- [ ] MessageService实现
- [ ] 数据库集成
- [ ] UI ViewModel对接

**Day 11-12**: 集成测试和Code Review
- [ ] 双实例聊天测试
- [ ] Code Review修复
- [ ] 文档更新

**Day 13-14**: Sprint Review和Retrospective
- [ ] Demo准备
- [ ] Sprint回顾会议
- [ ] Sprint 3规划

---

## 🎯 每日目标（Daily Goals）

### 2024-11-19 (Day 1)

- [x] Sprint Planning会议
- [x] 创建ADR-001: 架构现代化决策
- [x] 创建CMakePresets.json
- [x] 创建接口抽象层定义
- [x] 创建protocol/messages.proto框架
- [x] 更新Sprint 2 plan.md

### 2024-11-20 (Day 2)

- [ ] 完善protocol/messages.proto
- [ ] CMake集成protoc
- [ ] 生成.pb.cc/.pb.h验证
- [ ] 创建ProtobufSerializer.h/.cpp

### 2024-11-21 (Day 3)

- [ ] 实现ProtobufSerializer::serializePeerAnnounce()
- [ ] 实现ProtobufSerializer::deserializePeerMessage()
- [ ] 单元测试：节点发现消息

### 2024-11-22 (Day 4)

- [ ] 实现ProtobufSerializer::serializeTextMessage()
- [ ] 单元测试：文本消息
- [ ] 性能基准测试

### 2024-11-23 (Day 5)

- [ ] 创建QtNetworkAdapter.h/.cpp
- [ ] 实现UDP操作（bindUdp, sendBroadcast, receiveUdp）
- [ ] 单元测试：UDP操作

### 2024-11-24 (Day 6) - Weekend

- [ ] 实现QtNetworkAdapter TCP操作
- [ ] 单元测试：TCP操作

### 2024-11-25 (Day 7)

- [ ] 创建TcpConnectionManager.h/.cpp
- [ ] 实现connectToPeer()
- [ ] 实现disconnectPeer()

### 2024-11-26 (Day 8)

- [ ] 实现心跳机制
- [ ] 实现重连逻辑
- [ ] 单元测试：连接管理

### 2024-11-27 (Day 9)

- [ ] 集成测试：双实例TCP连接
- [ ] 长时间运行稳定性测试
- [ ] Code Review: TcpConnectionManager

### 2024-11-28 (Day 10)

- [ ] 创建MessageService.h/.cpp
- [ ] 实现sendTextMessage()
- [ ] 实现消息接收处理

### 2024-11-29 (Day 11)

- [ ] 数据库Schema创建
- [ ] 消息持久化实现
- [ ] ViewModel对接

### 2024-11-30 (Day 12)

- [ ] 集成测试：双实例聊天
- [ ] 离线消息测试
- [ ] Code Review: MessageService

### 2024-12-01 (Day 13) - Weekend

- [ ] Demo准备
- [ ] 文档更新（README, ADR）
- [ ] Sprint Review材料

### 2024-12-02 (Day 14)

- [ ] Sprint Review会议
- [ ] Sprint Retrospective
- [ ] Sprint 3规划启动

---

## ⚠️ 风险与应对

### 风险1: Protobuf学习曲线

**概率**: 中  
**影响**: 中（可能延迟2-3天）

**缓解措施**:
- Day 1完成技术验证（生成代码可编译）
- 准备降级方案（保留文本协议解析代码）
- 参考官方Tutorial: https://protobuf.dev/getting-started/cpptutorial/

**应急预案**:
- 如Day 3仍未完成验证，立即启用降级方案
- Sprint 3再次尝试Protobuf

### 风险2: TCP连接稳定性

**概率**: 低  
**影响**: 高（影响聊天功能）

**缓解措施**:
- 充分测试重连机制
- 心跳检测避免假死连接
- 集成测试覆盖异常场景（网络中断、对方崩溃）

**应急预案**:
- 保留Debug日志用于排查
- 延长测试时间（Day 9-10）

### 风险3: 任务估算偏差

**概率**: 中  
**影响**: 中（完成度<90%）

**缓解措施**:
- Sprint容量预留1 SP缓冲（已消耗于Tech-001/003）
- 每日Stand-up及时识别延迟
- P1任务可推迟到Sprint 3

**应急预案**:
- US-004优先级高于Tech-003，如时间不足优先US-004
- Tech-003（接口抽象）可分批重构

---

## 📈 定义完成（Definition of Done）

### Sprint级别DoD

- [ ] 所有P0 User Stories完成
- [ ] 代码通过Code Review（评分≥9.0）
- [ ] 单元测试覆盖率≥85%
- [ ] 集成测试全部通过
- [ ] 文档更新（README, ADR, 代码注释）
- [ ] 无Critical/Major Bug
- [ ] Demo验证功能可用

### Story级别DoD

- [ ] 所有验收标准通过
- [ ] 代码格式化（clang-format）
- [ ] 静态分析通过（clang-tidy）
- [ ] 单元测试覆盖率≥85%
- [ ] Doxygen注释完整
- [ ] 无内存泄漏（Valgrind）

---

## 🎓 培训与知识分享

### Sprint 2 Kick-off培训

**时间**: 2024-11-19 14:00  
**时长**: 90分钟

**议程**:
1. CMake Presets基础（30min）
   - 配置Preset vs 构建Preset
   - 继承机制
   - 条件编译

2. Protobuf C++ Tutorial（30min）
   - .proto语法
   - protoc代码生成
   - 序列化/反序列化API

3. 六边形架构实践（30min）
   - 接口抽象的价值
   - Mock测试示例
   - 依赖注入模式

**资料**:
- [CMake Presets文档](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- [Protobuf C++ Tutorial](https://protobuf.dev/getting-started/cpptutorial/)
- [ADR-001: 架构现代化](../../architecture/adr/001-architecture-modernization.md)

---

## 📊 Sprint燃尽图

| 日期 | 剩余SP | 理想剩余 | 实际进度 |
|------|--------|----------|----------|
| Day 0 (11-19) | 17 | 17 | Sprint开始 |
| Day 1 (11-20) | 15 | 15.7 | Tech-001/003完成 |
| Day 2 (11-21) | TBD | 14.4 | 待更新 |
| Day 3 (11-22) | TBD | 13.1 | 待更新 |
| Day 4 (11-23) | TBD | 11.8 | 待更新 |
| Day 5 (11-24) | TBD | 10.5 | 待更新 |
| Day 6 (11-25) | TBD | 9.2 | 待更新 |
| Day 7 (11-26) | TBD | 7.9 | 待更新 |
| Day 8 (11-27) | TBD | 6.6 | 待更新 |
| Day 9 (11-28) | TBD | 5.3 | 待更新 |
| Day 10 (11-29) | TBD | 4.0 | 待更新 |
| Day 11 (11-30) | TBD | 2.7 | 待更新 |
| Day 12 (12-01) | TBD | 1.4 | 待更新 |
| Day 13 (12-02) | 0 | 0 | Sprint结束 |

**更新规则**: 每日Stand-up后更新实际剩余SP

---

## 🔗 相关文档

- [ADR-001: 架构现代化决策](../../architecture/adr/001-architecture-modernization.md)
- [架构重构路线图](../../architecture/REFACTORING_ROADMAP.md)
- [Sprint 1回顾](../sprint_1/retrospective.md)
- [US-002: TCP长连接管理器](../../requirements/backlog/US-002_tcp-connection-manager.md) ✅
- [TechSpec-002: TCP连接管理器技术规格](../../requirements/backlog/TechSpec-002_tcp-connection-manager.md) ✅
- [US-004: 文本消息与聊天窗口](../../requirements/backlog/US-004_text-messaging.md) ✅
- [TechSpec-004: 文本消息技术规格](../../requirements/backlog/TechSpec-004_text-messaging.md) ✅

---

**计划创建日期**: 2024-11-19  
**负责人**: Sprint Planning Agent  
**审核人**: Orchestrator Agent, Architecture Design Agent  
**批准日期**: 2024-11-19
