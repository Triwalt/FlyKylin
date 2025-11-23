# FlyKylin 架构重构路线图

**创建日期**: 2024-11-19  
**负责人**: Architecture Design Agent  
**关联ADR**: [ADR-001](./adr/001-architecture-modernization.md)

---

## 📋 执行摘要

本文档定义FlyKylin项目从"传统C++/Qt架构"向"现代AI友好架构"演进的完整路线图。基于Sprint 1的扎实基础（代码质量9.2/10，测试覆盖100%），我们将在4个Sprint（8周）内完成架构现代化，目标是：

✅ **可测试性**: 核心逻辑纯净化，测试覆盖率≥90%  
✅ **AI友好性**: QML声明式UI，Protobuf结构化协议  
✅ **开发效率**: Docker环境，构建时间从20min→2min  
✅ **代码质量**: 整洁架构，协程替代回调地狱

---

## 🎯 核心目标

### 1. 架构层面
- **从**: 三层架构（UI→Core→Data）+ Qt强绑定
- **到**: 六边形架构（Domain←Ports→Adapters）+ 纯C++核心

### 2. 异步模型
- **从**: 信号槽回调链（callback hell）
- **到**: C++20协程线性代码（co_await）

### 3. GUI层面
- **从**: C++ Widgets硬编码布局
- **到**: QML声明式UI + 热重载

### 4. 构建系统
- **从**: PowerShell脚本 + 手动配置
- **到**: CMakePresets.json + Docker容器

### 5. 通信协议
- **从**: 文本协议（技术债务TD-001）
- **到**: Protobuf结构化 + 版本兼容

---

## 📊 实施时间线

### Sprint 2: 基础设施现代化 (Week 1-2)

**日期**: 2024-11-19 ~ 2024-12-02  
**主题**: "建立新架构基石，不影响现有功能"  

| 任务 | 工作量 | 优先级 | 依赖 |
|-----|--------|--------|------|
| CMakePresets.json | 4h | P0 | 无 |
| Protobuf协议定义 | 8h | P0 | vcpkg |
| 接口抽象层（I_NetworkAdapter） | 6h | P0 | 无 |
| US-002: TCP长连接 | 6h | P1 | Protobuf |
| US-004: 文本消息收发 | 8h | P1 | TCP |

**交付成果**:
- ✅ `cmake --preset windows-release` 一键构建
- ✅ protocol/messages.proto定义完成
- ✅ I_NetworkAdapter/I_MessageSerializer接口
- ✅ TCP长连接实现并测试
- ✅ 1v1文本聊天功能

**成功标准**:
```bash
# 构建验证
cmake --preset windows-release
cmake --build build/windows-release
./build/windows-release/bin/FlyKylin.exe

# 功能测试
- UDP发现正常 ✅
- TCP连接稳定 ✅
- Protobuf通信正常 ✅
- 1v1聊天可用 ✅
```

### Sprint 3: QML UI迁移 (Week 3-4)

**日期**: 2024-12-03 ~ 2024-12-16  
**主题**: "声明式UI，AI生成友好"

| 任务 | 工作量 | 优先级 | 依赖 |
|-----|--------|--------|------|
| Qt Quick依赖集成 | 2h | P0 | vcpkg.json |
| Main.qml主框架 | 6h | P0 | Qt Quick |
| PeerList.qml组件 | 8h | P0 | ViewModel |
| ChatView.qml组件 | 10h | P1 | ViewModel |
| Material风格样式 | 4h | P2 | QML组件 |

**交付成果**:
- ✅ PeerListWidget → PeerList.qml（代码量-60%）
- ✅ ChatView QML组件
- ✅ Material Design风格
- ✅ 热重载开发体验

**成功标准**:
```qml
// PeerList.qml 示例
ListView {
    model: peerListViewModel
    delegate: ItemDelegate {
        text: model.userName
        icon.source: model.avatar
        onClicked: peerListViewModel.selectPeer(model.userId)
    }
}
```

**对比指标**:
| 指标 | Widgets版本 | QML版本 | 改进 |
|-----|------------|---------|------|
| 代码行数 | ~200行 | ~80行 | -60% |
| 构建时间 | 需重编译 | 热重载 | 秒级 |
| AI生成成功率 | ~60% | ~90% | +30% |

### Sprint 4: 协程重构 (Week 5-6)

**日期**: 2024-12-17 ~ 2024-12-30  
**主题**: "线性异步，告别回调地狱"

| 任务 | 工作量 | 优先级 | 依赖 |
|-----|--------|--------|------|
| QCoro6集成 | 4h | P0 | vcpkg |
| PeerDiscovery协程化 | 8h | P0 | QCoro |
| TCP通信协程化 | 8h | P0 | QCoro |
| 文件传输协程化 | 8h | P1 | QCoro |
| 协程测试框架 | 4h | P1 | GoogleTest |

**交付成果**:
- ✅ 核心服务全部协程化
- ✅ 代码可读性显著提升
- ✅ 错误处理简化
- ✅ 测试覆盖率≥85%

**代码对比**:
```cpp
// 重构前：回调地狱
connect(socket, &QTcpSocket::readyRead, [=]() {
    auto data = socket->readAll();
    connect(parser, &Parser::parsed, [=](Message msg) {
        connect(handler, &Handler::processed, [=](Result r) {
            emit messageReceived(r);
        });
        handler->process(msg);
    });
    parser->parse(data);
});

// 重构后：协程线性代码
Task<void> receiveMessage() {
    auto data = co_await socket->readAsync();
    auto msg = co_await parser->parseAsync(data);
    auto result = co_await handler->processAsync(msg);
    co_await notifyMessageReceived(result);
}
```

### Sprint 5: Docker容器化 (Week 7-8)

**日期**: 2024-12-31 ~ 2025-01-13  
**主题**: "环境一致，极速构建"

| 任务 | 工作量 | 优先级 | 依赖 |
|-----|--------|--------|------|
| Dockerfile开发镜像 | 8h | P0 | Ubuntu 24.04 |
| CI/CD Docker集成 | 6h | P0 | GitHub Actions |
| devcontainer.json | 4h | P1 | VS Code |
| vcpkg二进制缓存 | 4h | P1 | Docker |
| 文档和培训 | 4h | P2 | 无 |

**交付成果**:
- ✅ flykylin-dev Docker镜像（<2GB）
- ✅ GitHub Actions使用Docker构建
- ✅ VS Code一键容器开发
- ✅ 构建时间≤5min

**环境对比**:
| 阶段 | 环境配置时间 | 构建时间 | 一致性 |
|-----|------------|---------|--------|
| Sprint 1 | 4小时 | 20分钟 | 低（手动） |
| Sprint 5 | 10分钟 | 5分钟 | 高（Docker） |

**Docker使用流程**:
```bash
# 开发者入职
git clone https://github.com/flykylin/flykylin.git
cd flykylin
code .  # VS Code自动提示打开容器

# 构建和运行
cmake --preset linux-release
cmake --build build/linux-release
```

---

## 🔄 重构策略

### 渐进式演进 (Strangler Fig Pattern)

```
Sprint 1 (已完成)          Sprint 2-5 (重构中)         Sprint 6+ (完成)
┌─────────────┐            ┌─────────────┐            ┌─────────────┐
│  Widgets UI │            │  QML UI     │            │  QML UI     │
│             │            │  (新增)      │            │             │
│  PeerList   │  ──────>   │  PeerList   │  ──────>   │  ChatView   │
│  Widget     │            │  .qml       │            │  .qml       │
│  (保留)      │            │             │            │             │
├─────────────┤            ├─────────────┤            ├─────────────┤
│  Core       │            │  Domain     │            │  Domain     │
│  (Qt依赖)    │  ──────>   │  (纯C++)    │  ──────>   │  (协程)      │
│             │            │  + Adapters │            │  + Ports    │
├─────────────┤            ├─────────────┤            ├─────────────┤
│  文本协议    │            │  Protobuf   │            │  Protobuf   │
│  (TD-001)   │  ──────>   │  (已实现)    │  ──────>   │  (稳定)      │
└─────────────┘            └─────────────┘            └─────────────┘
```

### 降级方案

每个Sprint保留降级路径，确保可随时回退：

| Sprint | 新特性 | 降级方案 |
|--------|--------|---------|
| Sprint 2 | Protobuf | 保留文本协议解析代码 |
| Sprint 3 | QML UI | 保留Widgets版本（条件编译） |
| Sprint 4 | 协程 | 保留信号槽版本 |
| Sprint 5 | Docker | 本地构建仍可用 |

---

## 📈 验证指标

### 定量指标

#### 开发效率
| 指标 | 基线（Sprint 1） | 目标（Sprint 5） | 测量方法 |
|-----|----------------|----------------|---------|
| 构建时间 | 20分钟 | ≤5分钟 | CI日志 |
| 环境搭建 | 4小时 | ≤10分钟 | 新人反馈 |
| UI修改编译 | 需重编译 | 热重载（秒级） | 实测 |
| AI生成成功率 | 60% | ≥85% | Code Review |

#### 代码质量
| 指标 | 基线（Sprint 1） | 目标（Sprint 5） | 测量方法 |
|-----|----------------|----------------|---------|
| 测试覆盖率 | 100%（核心） | ≥90%（全项目） | gcov |
| 圈复杂度 | 12（平均） | ≤8 | lizard |
| 代码行数 | 3000行 | ~2500行 | cloc |
| 重复代码率 | 5% | ≤3% | cpd |

#### 性能
| 指标 | 基线（Sprint 1） | 目标（Sprint 5） | 测量方法 |
|-----|----------------|----------------|---------|
| 启动时间 | <2秒 | ≤2秒 | 性能测试 |
| 节点发现 | <5秒 | ≤5秒 | 集成测试 |
| UI响应 | <100ms | ≤100ms | UI测试 |
| 内存占用 | ~50MB | ≤60MB | 内存分析 |

### 定性指标

#### 架构健康度检查表

**Sprint 2完成时**:
- [ ] 所有网络操作使用I_NetworkAdapter接口
- [ ] 所有消息使用Protobuf序列化
- [ ] CMake配置无环境变量依赖
- [ ] 技术债务TD-001已清理

**Sprint 3完成时**:
- [ ] 主UI使用QML实现
- [ ] ViewModel完全分离业务逻辑
- [ ] QML代码通过热重载验证
- [ ] Widgets版本作为降级方案保留

**Sprint 4完成时**:
- [ ] 异步操作≥60%使用协程
- [ ] 无嵌套超过3层的回调
- [ ] 协程错误处理统一
- [ ] 协程测试覆盖≥85%

**Sprint 5完成时**:
- [ ] CI使用Docker镜像构建
- [ ] 本地开发可用devcontainer
- [ ] 构建时间≤5分钟
- [ ] 工具脚本减少≥90%

---

## ⚠️ 风险管理

### 高风险项

#### 风险1: Protobuf集成复杂度
**概率**: 中  
**影响**: 高（阻塞Sprint 2）  

**缓解措施**:
1. Sprint 2第1周完成技术验证
2. 准备降级方案（保留文本协议）
3. vcpkg确保Protobuf版本兼容

**应急预案**:
- 如第1周未完成验证，立即降级到文本协议
- Sprint 3再次尝试Protobuf

#### 风险2: QCoro库稳定性
**概率**: 低  
**影响**: 高（影响Sprint 4）

**缓解措施**:
1. 使用QCoro6稳定版本
2. 先在小范围试点（PeerDiscovery）
3. 充分测试边界情况

**应急预案**:
- 保留信号槽版本作为条件编译选项
- 如严重问题，Sprint 5继续优化

#### 风险3: Docker镜像过大
**概率**: 中  
**影响**: 中（影响CI性能）

**缓解措施**:
1. 使用多阶段构建
2. 清理apt缓存
3. 优化vcpkg依赖

**应急预案**:
- 目标2GB，可接受上限3GB
- 使用Docker Hub缓存加速

### 中风险项

#### 风险4: 学习曲线影响进度
**概率**: 高  
**影响**: 中（延长迭代时间）

**缓解措施**:
1. 每Sprint重点引入1-2个新技术
2. 提供培训文档和示例
3. Code Review分享最佳实践

**应急预案**:
- Sprint容量预留20%缓冲
- 可推迟P2优先级任务

---

## 📚 技术债务管理

### 新增技术债务

| ID | 描述 | 引入Sprint | 计划清理 | 优先级 |
|----|------|-----------|---------|--------|
| TD-002 | Widgets版本维护成本 | Sprint 3 | Sprint 4完成后移除 | Low |
| TD-003 | 信号槽与协程混用 | Sprint 4 | Sprint 5统一协程 | Medium |
| TD-004 | 本地vcpkg依赖 | Sprint 5 | 未来迁移到Conan | Low |

### 清理的技术债务

| ID | 描述 | 引入时间 | 清理Sprint | 验证 |
|----|------|---------|-----------|------|
| TD-001 | 文本协议 | Sprint 1 | Sprint 2 | ✅ Protobuf替换 |

---

## 🎓 培训计划

### Sprint 2培训：CMake & Protobuf

**时间**: Sprint 2第1天  
**内容**:
1. CMake Presets基础（30min）
2. Protobuf定义和生成（30min）
3. 接口抽象最佳实践（30min）

**资料**:
- [CMake Presets文档](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- [Protobuf C++ Tutorial](https://protobuf.dev/getting-started/cpptutorial/)
- [六边形架构示例](./adr/001-architecture-modernization.md#决策)

### Sprint 3培训：QML开发

**时间**: Sprint 3第1天  
**内容**:
1. QML语法和组件（45min）
2. C++ ViewModel与QML绑定（30min）
3. Material Design实践（15min）

**资料**:
- [Qt Quick入门](https://doc.qt.io/qt-6/qtquick-index.html)
- [QML最佳实践](https://doc.qt.io/qt-6/qtquick-bestpractices.html)

### Sprint 4培训：C++20协程

**时间**: Sprint 4第1天  
**内容**:
1. C++20协程基础（45min）
2. QCoro使用方法（30min）
3. 协程错误处理（15min）

**资料**:
- [C++20 Coroutines](https://en.cppreference.com/w/cpp/language/coroutines)
- [QCoro文档](https://qcoro.dvratil.cz/)

### Sprint 5培训：Docker开发

**时间**: Sprint 5第1天  
**内容**:
1. Docker基础概念（30min）
2. devcontainer使用（30min）
3. 本地调试技巧（30min）

**资料**:
- [Docker文档](https://docs.docker.com/)
- [Dev Containers](https://code.visualstudio.com/docs/devcontainers/containers)

---

## ✅ 验收标准

### Sprint 2交付验收

```bash
# 1. 构建系统
✅ cmake --preset windows-release（成功）
✅ cmake --build build/windows-release（成功）
✅ 无需设置环境变量

# 2. Protobuf通信
✅ 运行PeerDiscovery单元测试（全部通过）
✅ 抓包验证Protobuf格式
✅ 版本兼容性测试

# 3. 接口抽象
✅ I_NetworkAdapter单元测试
✅ Mock测试覆盖率≥90%
✅ QtNetworkAdapter集成测试

# 4. 功能验证
✅ UDP发现正常（双实例测试）
✅ TCP连接稳定（长时间运行）
✅ 1v1聊天可用（发送/接收）
```

### Sprint 3交付验收

```bash
# 1. QML UI
✅ Main.qml启动成功
✅ PeerList.qml显示在线用户
✅ 热重载功能验证

# 2. 功能等价性
✅ 在线用户列表（与Widgets版本对比）
✅ 搜索过滤功能
✅ 用户选择交互

# 3. 性能
✅ UI响应时间<100ms
✅ 内存占用≤60MB
✅ RK3566平台测试通过
```

### Sprint 4交付验收

```bash
# 1. 协程实现
✅ PeerDiscovery协程化
✅ TCP通信协程化
✅ 文件传输协程化（如已实现）

# 2. 代码质量
✅ 圈复杂度≤8
✅ 协程测试覆盖率≥85%
✅ Code Review评分≥9.5

# 3. 性能
✅ 无性能回归
✅ 错误处理完善
✅ 稳定性测试通过
```

### Sprint 5交付验收

```bash
# 1. Docker镜像
✅ docker pull flykylin/dev（成功）
✅ 镜像大小≤2GB
✅ VS Code devcontainer打开成功

# 2. CI/CD
✅ GitHub Actions使用Docker
✅ 构建时间≤5分钟
✅ 构建成功率≥95%

# 3. 文档
✅ Docker使用文档完整
✅ 新人上手指南
✅ 常见问题FAQ
```

---

## 📝 相关文档

### 核心文档
- [ADR-001: 架构现代化决策](./adr/001-architecture-modernization.md)
- [Sprint 1回顾](../sprints/active/sprint_1/retrospective.md)
- [项目架构规则](./.windsurf/rules/project-architecture.md)

### 技术文档
- [CMake Presets规范](./build/CMAKE_PRESETS.md)（待创建）
- [Protobuf协议定义](./protocol/README.md)（待创建）
- [QML组件库](./ui/qml/README.md)（待创建）
- [协程最佳实践](./async/COROUTINE_GUIDE.md)（待创建）
- [Docker开发指南](./docker/DEV_GUIDE.md)（待创建）

---

## 📞 联系方式

**架构问题**: Architecture Design Agent  
**Sprint规划**: Sprint Planning Agent  
**技术实现**: Development Execution Agent  
**质量监控**: Quality Assurance Agent

---

**文档版本**: v1.0  
**最后更新**: 2024-11-19  
**下次审核**: Sprint 2结束后
