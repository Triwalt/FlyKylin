# Sprint 1 需求分析报告

**日期**: 2025-11-18  
**分析人**: Cascade AI  
**状态**: ✅ 已完成

---

## 📋 分析摘要

基于FlyKylin项目整体方案和Sprint 0环境验证成功，完成了Sprint 1的需求分析，创建了3个User Story，总计18个故事点。

## 🎯 Sprint 1目标

**实现局域网P2P节点自动发现和基础文本通信**

### 核心价值
- 验证P2P通信技术方案可行性
- 为后续功能打下通信基础
- 实现MVP（最小可行产品）演示

## 📝 User Stories

### US-001: UDP节点发现
- **优先级**: Critical
- **故事点**: 5
- **核心功能**: UDP广播节点发现，自动建立在线用户列表
- **技术重点**: QUdpSocket、Protobuf消息序列化
- **风险**: UDP广播可能被防火墙拦截

### US-002: TCP长连接消息传输
- **优先级**: Critical
- **故事点**: 8
- **核心功能**: TCP长连接、文本消息收发、心跳保活
- **技术重点**: QTcpSocket、长度前缀协议、断线重连
- **风险**: TCP粘包处理、连接泄漏

### US-003: 基础UI用户列表
- **优先级**: High
- **故事点**: 5
- **核心功能**: 在线用户列表、用户搜索、双击打开聊天
- **技术重点**: MVVM模式、QListView、信号槽
- **风险**: 大量用户时UI性能

## 🔍 技术决策

### 通信协议简化
- ✅ UDP明文广播（无加密）
- ✅ TCP明文传输（无TLS）
- ✅ IP+主机名作为身份（无认证）
- **理由**: 局域网可信环境，性能和开发效率优先

### 消息序列化
- ✅ 选择Protobuf
- **优势**: 性能高、跨平台、类型安全
- **依赖**: 需要安装完整Protobuf库（vcpkg）

### UI架构
- ✅ MVVM模式
- ✅ Qt Widgets (非QML)
- **理由**: 成熟稳定、RK3566性能考虑

## 📊 工作量估算

| Story | 设计 | 开发 | 测试 | 总计 |
|-------|------|------|------|------|
| US-001 | 0.5天 | 2天 | 1天 | 3.5天 |
| US-002 | 1天 | 3天 | 1.5天 | 5.5天 |
| US-003 | 0.5天 | 2天 | 1天 | 3.5天 |
| **合计** | **2天** | **7天** | **3.5天** | **12.5天** |

**Sprint周期**: 2周（10个工作日）
**缓冲**: 留有2.5天处理技术债务和风险

## ⚠️ 关键依赖

### 前置条件
- [x] Sprint 0环境验证完成
- [x] Qt、MSVC、ONNX Runtime已配置
- [ ] Protobuf完整库安装（vcpkg）
- [ ] GoogleTest测试框架集成

### 技术债务
1. **Protobuf安装**: 需要通过vcpkg安装完整C++库
2. **日志系统**: 需要实现基础日志框架（spdlog）
3. **测试框架**: 配置GoogleTest和集成测试环境

## 🎯 验收标准

### Sprint目标验收
- [ ] 启动2个FlyKylin实例
- [ ] 自动发现并在UI显示对方
- [ ] 双击用户名打开聊天窗口
- [ ] 发送文本消息"Hello from Sprint 1!"
- [ ] 对方接收并显示消息

### 性能目标
- [ ] 节点发现延迟 < 5秒
- [ ] 消息传输延迟 < 100ms
- [ ] UI响应时间 < 100ms
- [ ] 单元测试覆盖率 ≥ 70%

## 📈 后续规划

### Sprint 2预览
- 用户信息管理（本地配置）
- 分组管理
- 完整聊天窗口UI
- 消息历史记录（SQLite）

### 技术演进
- Sprint 3: 群聊、文件传输
- Sprint 4: RK3566跨平台
- Sprint 5: AI NSFW检测
- Sprint 6: AI语义搜索

## 📚 交付物清单

### 需求文档
- [x] `US-001_UDP节点发现.md`
- [x] `US-002_TCP长连接.md`
- [x] `US-003_基础UI.md`
- [x] `Sprint_1_Plan.md`
- [x] `Sprint_1_Analysis.md` (本文档)

### 技术设计
- [x] Protobuf消息定义
- [x] 核心类设计（PeerDiscovery, PeerSession, CommunicationService）
- [x] MVVM架构设计（PeerListViewModel, PeerListWidget）

### 下一步行动
1. 运行 `install-protobuf-vcpkg.cmd` 安装Protobuf
2. 配置GoogleTest测试框架
3. 创建`src/core/communication/`目录结构
4. 实现US-001 UDP节点发现

## ✅ 需求分析检查清单

- [x] User Story符合INVEST原则
- [x] 验收标准具体可测试
- [x] 技术可行性已验证
- [x] 性能指标明确
- [x] 跨平台兼容性考虑（Windows优先）
- [x] 依赖关系已识别
- [x] 风险已评估
- [x] 故事点估算合理

---

**需求分析完成，可以开始Sprint 1开发！** 🚀
