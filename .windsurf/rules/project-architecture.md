---
trigger: always_on
description: FlyKylin项目架构设计原则和约束
---

<architecture_principles>

# 分层架构

## 三层架构
- **UI层**：Qt Widgets/QML界面，负责用户交互
- **业务逻辑层**：核心服务（通信、AI、数据管理）
- **数据访问层**：SQLite数据库、文件存储

## 模块划分
- **core/**：核心功能模块（P2P、消息、加密）
- **ai/**：AI引擎（NSFW检测、翻译、摘要、语义搜索）
- **platform/**：平台相关代码（Windows/RK3566加速器实现）
- **ui/**：界面层（ViewModel、View）
- **storage/**：数据持久化（数据库、文件管理）

# 设计模式

## 必须使用的模式
- **单例模式**：全局服务（AIEngine、DatabaseManager）
- **工厂模式**：加速器创建（根据平台选择DirectML/NPU）
- **观察者模式**：Qt信号槽实现事件驱动
- **策略模式**：AI模型切换（不同任务使用不同模型）
- **MVVM模式**：UI层使用ViewModel分离逻辑

## 禁止使用的模式
- 避免过度使用继承，优先组合
- 避免上帝类（God Class）
- 避免循环依赖

# 依赖管理

## 依赖方向
- UI层 → 业务逻辑层 → 数据访问层（单向依赖）
- 平台相关代码实现抽象接口，不暴露平台细节
- 使用接口（`I_` 前缀）解耦模块

## 第三方库使用
- Qt仅用于UI、网络、并发、数据库
- ONNX Runtime仅在AI模块使用
- Protobuf仅在通信层使用
- 禁止跨层直接使用第三方库

# 性能约束

## Windows平台
- 启动时间 < 2秒
- UI响应 < 100ms
- P2P发现延迟 < 5秒
- AI推理（NSFW） < 500ms

## RK3566平台
- 启动时间 < 5秒
- UI响应 < 200ms
- NPU推理（NSFW） < 1秒
- 内存占用 < 200MB

# 跨平台兼容

## 平台抽象
- 使用 `I_Accelerator` 接口抽象AI加速
- Windows实现：`DirectMLAccelerator`
- RK3566实现：`RKNPUAccelerator`
- 编译时根据平台选择实现

## 平台特定代码
- 使用条件编译 `#ifdef Q_OS_WIN` / `#ifdef RK3566_PLATFORM`
- 平台代码隔离在 `platform/` 目录
- 核心逻辑保持平台无关

# 并发模型

## 线程使用规范
- **主线程**：UI操作、事件循环
- **网络线程**：P2P发现、消息收发
- **AI线程**：AI推理（ONNX Runtime）
- **数据库线程**：SQLite操作

## 线程安全
- 跨线程通信使用Qt信号槽（Queued连接）
- 共享数据使用互斥锁保护
- 避免死锁：固定加锁顺序

# 错误处理策略

## 分级处理
- **Critical**：崩溃级别，记录并退出
- **Error**：功能失败，记录并提示用户
- **Warning**：非致命问题，记录日志
- **Info**：调试信息

## 用户友好
- 不向用户显示技术细节
- 提供可操作的错误提示
- 关键操作提供重试机制

# 测试要求

## 单元测试
- 核心业务逻辑覆盖率 ≥ 80%
- 使用GoogleTest框架
- Mock外部依赖（网络、文件系统）

## 集成测试
- P2P发现测试（模拟多节点）
- AI推理测试（双平台）
- 数据库迁移测试

## 性能测试
- 启动时间基准测试
- AI推理性能测试
- 内存泄漏检测（Valgrind）

</architecture_principles>

<sprint_constraints>

# Sprint规划约束

## Sprint周期
- 每个Sprint为2周
- 每Sprint完成1-3个User Story
- 每Sprint必须可演示

## Definition of Done
- [ ] 代码通过Code Review
- [ ] 单元测试通过且覆盖率达标
- [ ] 集成测试通过
- [ ] 文档更新完成
- [ ] 双平台验证通过

## 优先级原则
- P0：核心功能，必须完成
- P1：重要功能，Sprint内完成
- P2：增强功能，可推迟
- P3：优化改进，按需安排

</sprint_constraints>