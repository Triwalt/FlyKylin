# Sprint 1 Plan - P2P通信基础

**Sprint周期**: 2024-11-18 ~ 2024-12-01 (2周)  
**Team Capacity**: 25故事点  
**实际纳入**: 18故事点  
**状态**: 规划中

---

## 🎯 Sprint目标

**实现局域网P2P自动发现和基础文本消息通信，完成Windows平台MVP演示。**

通过本Sprint，用户可以：
1. 启动多个FlyKylin实例自动发现彼此
2. 选择在线用户发送文本消息
3. 实时接收和显示消息

## 📋 纳入的User Stories

### US-001: UDP节点发现 (Critical, 5点)

**目标**: 实现UDP广播节点发现和在线用户列表维护

**验收标准**:
- [ ] AC1: 应用启动后每5秒发送UDP广播
- [ ] AC2: 接收其他节点广播并添加到在线列表
- [ ] AC3: 30秒未收到心跳则标记节点离线
- [ ] AC4: 支持同时发现5+个在线节点

**任务分解**:
1. [ ] T1.1 - 设计Protobuf消息结构 (2h)
2. [ ] T1.2 - 实现PeerNode数据模型 (2h)
3. [ ] T1.3 - 实现PeerDiscovery类（UDP广播） (6h)
4. [ ] T1.4 - 实现心跳超时检测逻辑 (4h)
5. [ ] T1.5 - 编写单元测试 (4h)
6. [ ] T1.6 - 集成测试（2个实例互相发现） (2h)

**总工时**: 20h  
**依赖**: Protobuf安装完成  
**风险**: Medium - UDP可能被防火墙拦截

---

### US-002: TCP长连接消息传输 (Critical, 8点)

**目标**: 建立TCP长连接实现可靠文本消息收发

**验收标准**:
- [ ] AC1: 成功建立TCP连接到发现的节点
- [ ] AC2: 发送文本消息通过Protobuf序列化
- [ ] AC3: 接收消息并正确解析显示
- [ ] AC4: 60秒无消息自动发送PING心跳
- [ ] AC5: 连接断开后3次重连尝试

**任务分解**:
1. [ ] T2.1 - 设计TCP消息协议（长度前缀） (3h)
2. [ ] T2.2 - 实现PeerSession类（单连接管理） (8h)
3. [ ] T2.3 - 实现CommunicationService（连接池） (6h)
4. [ ] T2.4 - 实现心跳和断线重连逻辑 (5h)
5. [ ] T2.5 - 处理TCP粘包问题 (4h)
6. [ ] T2.6 - 编写单元测试 (6h)
7. [ ] T2.7 - 集成测试（互发100条消息） (3h)
8. [ ] T2.8 - 性能测试（延迟<100ms） (2h)

**总工时**: 37h  
**依赖**: US-001完成（需要节点信息）  
**风险**: High - TCP粘包处理、连接泄漏

---

### US-003: 基础UI用户列表 (High, 5点)

**目标**: 实现在线用户列表UI和MVVM绑定

**验收标准**:
- [ ] AC1: 显示在线用户列表（用户名、IP、状态）
- [ ] AC2: 实时更新用户上线/离线
- [ ] AC3: 支持用户名搜索过滤
- [ ] AC4: 双击用户打开聊天窗口（占位）
- [ ] AC5: 系统托盘显示上线/离线通知

**任务分解**:
1. [ ] T3.1 - 设计UI布局（Qt Designer） (2h)
2. [ ] T3.2 - 实现PeerListViewModel（MVVM） (5h)
3. [ ] T3.3 - 实现PeerListWidget（视图） (5h)
4. [ ] T3.4 - 实现搜索过滤功能 (3h)
5. [ ] T3.5 - 集成CommunicationService信号 (3h)
6. [ ] T3.6 - 实现系统托盘通知 (2h)
7. [ ] T3.7 - UI测试（200+用户性能） (2h)

**总工时**: 22h  
**依赖**: US-001完成（数据源）  
**风险**: Low - UI性能问题可优化

---

## 📅 Sprint里程碑

### Week 1 (Day 1-5)

**Day 1-2**: 环境准备和基础设施
- [ ] 安装Protobuf完整库（vcpkg）
- [ ] 配置GoogleTest测试框架
- [ ] 创建项目目录结构
- [ ] 定义Protobuf消息格式

**Day 3**: US-001开发
- [ ] PeerDiscovery核心逻辑完成
- [ ] 单元测试通过

**Day 4-5**: US-002开发（第1阶段）
- [ ] PeerSession基础实现
- [ ] 消息收发测试通过

### Week 2 (Day 6-10)

**Day 6-7**: US-002完成 + US-003开始
- [ ] CommunicationService完成
- [ ] PeerListViewModel完成
- [ ] 心跳和重连逻辑完成

**Day 8**: US-003完成 + 集成
- [ ] UI完整实现
- [ ] 所有模块集成测试

**Day 9**: Code Review和修复
- [ ] Code Review完成
- [ ] 修复发现的问题
- [ ] 性能优化

**Day 10**: Sprint Review
- [ ] 准备演示环境
- [ ] Sprint Review演示
- [ ] Sprint Retrospective

---

## 🔧 技术依赖

### 必须完成
- [x] Qt 6.9.3安装
- [x] MSVC 2022配置
- [x] CMake项目配置
- [ ] Protobuf完整库（vcpkg install protobuf:x64-windows）
- [ ] GoogleTest框架（vcpkg install gtest:x64-windows）

### 可选但推荐
- [ ] spdlog日志库（vcpkg install spdlog:x64-windows）
- [ ] Catch2测试框架（备选）

---

## ⚠️ 风险和应对

### 风险1: Protobuf安装失败
- **影响**: Critical
- **概率**: Medium
- **应对**: 
  - 使用vcpkg自动安装
  - 提供手动下载编译文档
  - 最坏情况：暂时使用JSON替代

### 风险2: UDP广播被防火墙拦截
- **影响**: High
- **概率**: Medium
- **应对**:
  - 文档说明配置防火墙规则
  - 提供手动添加IP功能（Plan B）
  - 考虑mDNS作为备选方案（Sprint 2+）

### 风险3: TCP粘包导致消息解析错误
- **影响**: High
- **概率**: Medium  
- **应对**:
  - 严格使用长度前缀协议
  - 充分的边界测试（大量小消息）
  - Code Review重点检查

### 风险4: 跨平台测试延迟
- **影响**: Low（本Sprint仅Windows）
- **概率**: Low
- **应对**: Sprint 1聚焦Windows，RK3566延后到Sprint 4

---

## 📊 Definition of Done

**Story级别DoD**:
- [ ] 所有验收标准满足
- [ ] 代码通过Code Review
- [ ] 单元测试覆盖率 ≥ 70%
- [ ] 集成测试通过
- [ ] 无Critical/High Bug
- [ ] API文档完成

**Sprint级别DoD**:
- [ ] 演示场景可运行：
  - 启动2个FlyKylin实例
  - 自动发现并显示在线用户
  - 双击用户发送消息"Hello from Sprint 1!"
  - 对方接收并显示消息
- [ ] 性能目标达成：
  - 节点发现延迟 < 5秒
  - 消息传输延迟 < 100ms
  - UI响应 < 100ms
- [ ] 代码质量：
  - 整体测试覆盖率 ≥ 70%
  - 无内存泄漏（Valgrind验证）
  - Clang-tidy静态检查通过
- [ ] 文档更新：
  - API文档生成（Doxygen）
  - README更新
  - 用户指南更新

---

## 📈 Burndown跟踪

| Day | 计划剩余 | 实际剩余 | 状态 |
|-----|----------|----------|------|
| D0  | 18点 | 18点 | ⚪ 未开始 |
| D1  | 17点 | - | - |
| D2  | 15点 | - | - |
| D3  | 13点 | - | - |
| D4  | 10点 | - | - |
| D5  | 8点  | - | - |
| D6  | 6点  | - | - |
| D7  | 4点  | - | - |
| D8  | 2点  | - | - |
| D9  | 1点  | - | - |
| D10 | 0点  | - | - |

更新频率：每日Standup后更新

---

## 🎬 Sprint Review演示计划

**时间**: 2024-12-01, 30分钟  
**地点**: 线上/现场  
**参与**: 开发团队 + Stakeholders

### 演示脚本

**1. Sprint目标回顾** (2分钟)
- 目标：实现P2P通信MVP
- 完成Stories：3/3
- 完成故事点：18/18

**2. 功能演示** (20分钟)

**场景1: 节点自动发现** (5分钟)
```
操作：
1. 启动FlyKylin实例A
2. 等待3秒，启动实例B
3. 观察实例A自动显示实例B上线

验证：
- 发现延迟 < 5秒 ✓
- 用户列表显示正确信息 ✓
- 系统托盘通知 ✓
```

**场景2: 文本消息通信** (10分钟)
```
操作：
1. 在实例A双击实例B
2. 发送消息"Hello from Sprint 1!"
3. 实例B接收并显示

验证：
- 消息延迟 < 100ms ✓
- 消息内容正确 ✓
- 发送时间戳正确 ✓
```

**场景3: 节点离线检测** (5分钟)
```
操作：
1. 关闭实例B
2. 等待30秒
3. 观察实例A更新状态

验证：
- 30秒后标记离线 ✓
- UI状态更新 ✓
- 托盘通知 ✓
```

**3. 技术亮点** (5分钟)
- 完全去中心化P2P架构
- Protobuf高效序列化
- MVVM清晰架构
- 测试覆盖率达到75%

**4. 下一个Sprint预告** (3分钟)
- Sprint 2: 用户配置、分组管理、聊天窗口
- Sprint 3: 群聊、文件传输

---

## 📝 Daily Standup模板

使用`notes.md`记录每日进展，格式：

```markdown
## 2024-11-XX

### 开发者
- **昨天**: 完成Task X
- **今天**: 进行Task Y
- **阻碍**: 需要XXX支持

### Burndown
- 剩余: X点
- 风险: 无/有（描述）
```

---

## 📚 相关文档

- **User Stories**: `docs/requirements/backlog/US-00X.md`
- **需求分析**: `docs/requirements/Sprint_1_Analysis.md`
- **架构设计**: `docs/飞秋方案.md`
- **每日更新**: `docs/sprints/active/sprint_1/notes.md`

---

**准备就绪，Sprint 1开始！** 🚀
