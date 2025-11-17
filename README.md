# 智能飞秋项目 - 多Agent开发体系

> 基于AI Agent协作的下一代智能局域网协作套件

## 🎯 项目愿景

构建一个跨平台(Windows/RK3566)的局域网即时通讯工具，集成AI功能(NSFW检测、聊天摘要、语义搜索、消息翻译)并利用NPU/GPU硬件加速，同时建立基于多Agent协作的现代化软件开发工作流。

## 📚 文档导航

### 核心文档

1. **[飞秋方案.md](./飞秋方案.md)** - **必读** ⭐
   - 完整的技术方案
   - 系统架构设计
   - AI加速方案
   - 数据库设计
   - Sprint迭代计划

2. **[多Agent项目迭代工作流.md](./多Agent项目迭代工作流.md)** - **必读** ⭐
   - 9个专业Agent的职责划分
   - Agent协作流程
   - Sprint迭代工作流
   - 质量保证机制

3. **[agent_prompts.yaml](./agent_prompts.yaml)** - **配置文件** ⚙️
   - 每个Agent的详细提示词
   - 任务模板定义
   - 质量标准配置

4. **[项目总结与Agent使用指南.md](./项目总结与Agent使用指南.md)** - **快速入门** 🚀
   - 快速开始指南
   - Agent使用示例
   - 常见问题解答
   - 开发工具链

## 🏗️ 项目架构概览

```
智能飞秋 = 局域网P2P通信 + AI功能 + 硬件加速 + 多Agent开发流程

┌─────────────────────────────────────────────────────────┐
│                   应用层 (Qt/QML UI)                      │
├─────────────────────────────────────────────────────────┤
│              ViewModel层 (业务逻辑)                        │
├─────────────────────────────────────────────────────────┤
│        Model层 (通信 | AI引擎 | 数据存储)                  │
├─────────────────────────────────────────────────────────┤
│              硬件抽象层 (HAL)                              │
│   ┌──────────┬──────────┬──────────────┐                 │
│   │  NPU     │   GPU    │  CPU Fallback│                 │
│   │ (RK3566) │ (Windows)│              │                 │
│   └──────────┴──────────┴──────────────┘                 │
└─────────────────────────────────────────────────────────┘
```

## ✨ 核心特性

### 功能特性

- ✅ **P2P通信**：UDP广播节点发现 + TCP长连接通信
- ✅ **群聊功能**：支持全局广播和分组群发
- ✅ **文件传输**：支持文件/文件夹传输，断点续传
- ✅ **AI检测**：NSFW内容双端检测(发送前+接收后)
- ✅ **智能搜索**：基于语义的聊天记录搜索
- ✅ **消息翻译**：实时消息翻译
- ✅ **聊天摘要**：AI协同实现长对话摘要

### 技术特性

- 🚀 **跨平台**：Windows (x64) + Linux ARM64 (RK3566)
- 🧠 **AI加速**：NPU (RK3566) / GPU (Windows) / CPU Fallback
- 🏛️ **架构优秀**：MVVM模式 + 硬件抽象层 + 模块化设计
- 🤖 **AI开发流**：9个专业Agent协作开发
- 📊 **质量保证**：代码审查 + 自动化测试 + CI/CD

## 🤖 多Agent开发体系

### 9个专业Agent

| Agent | 职责 | 输出 |
|-------|------|------|
| 🎯 Orchestrator | 项目管理中枢 | Sprint计划、风险预警 |
| 📋 Requirements Analyst | 需求分析 | User Stories、验收标准 |
| 🏗️ Architecture Design | 架构设计 | 系统设计、接口规范 |
| 📅 Sprint Planning | Sprint规划 | 任务分解、时间线 |
| 💻 Development Execution | 开发执行 | 代码、单元测试 |
| 👀 Code Review | 代码审查 | 审查报告、改进建议 |
| 🧪 Testing | 测试 | 测试报告、Bug列表 |
| 📊 Quality Assurance | 质量监控 | 质量看板、技术债务 |
| 🚀 Build & Deployment | 构建部署 | 安装包、Release Notes |

### 协作流程

```
需求分析 → 架构设计 → Sprint规划 → 开发执行 
    ↓                                    ↓
质量监控 ← 测试验证 ← 代码审查 ←────────────┘
    ↓
构建部署
```

## 🚀 快速开始

### 1. 阅读文档

```bash
# 第一步：理解技术方案
阅读 "飞秋方案.md" 第1-4章

# 第二步：理解开发流程  
阅读 "多Agent项目迭代工作流.md"

# 第三步：查看快速入门
阅读 "项目总结与Agent使用指南.md"
```

### 2. 配置环境

```yaml
# 必需工具
- C++ Compiler: C++20 (MSVC/GCC)
- Qt: 6.x
- CMake: 3.20+
- ONNX Runtime: 1.14+
- rknn-toolkit2: 1.5+ (RK3566平台)
```

### 3. 启动第一个Sprint

参考 [项目总结与Agent使用指南.md](./项目总结与Agent使用指南.md) 中的"Step 3: 启动第一个Sprint"章节。

## 📊 技术栈

| 层次 | 技术选型 |
|-----|---------|
| 语言 | C++20 |
| UI框架 | Qt 6 (QML) |
| 网络 | Qt Network + Protobuf |
| AI推理 | ONNX Runtime + RKNPU |
| 数据库 | SQLite + FTS5 |
| 构建 | CMake + CPack |
| CI/CD | GitHub Actions |
| 测试 | GoogleTest + Catch2 |

## 📁 项目结构

```
ai-feiqiu-project/
├── README.md                          # 本文件
├── 飞秋方案.md                         # 技术方案(完整)
├── 多Agent项目迭代工作流.md             # Agent协作流程
├── agent_prompts.yaml                 # Agent配置
├── 项目总结与Agent使用指南.md          # 使用指南
├── docs/                              # 文档目录
│   ├── requirements/                  # 需求文档
│   ├── architecture/                  # 架构设计
│   ├── sprints/                       # Sprint记录
│   └── quality/                       # 质量报告
├── src/                               # 源代码
│   ├── core/                          # 核心模块(100%跨平台)
│   ├── platform/                      # 平台相关(AI加速)
│   ├── ui/                            # UI层
│   └── app/                           # 应用层
└── tests/                             # 测试代码
```

## 🎯 迭代计划

### Sprint 1: 核心P2P通信 (2周)
- ✅ UDP广播节点发现
- ✅ TCP长连接通信
- ✅ 1v1文本聊天

### Sprint 2: 用户体系与分组 (2周)
- ✅ 用户分组
- ✅ 在线用户搜索
- ✅ 好友列表

### Sprint 3: 群聊与文件 (2周)
- ✅ 群聊功能
- ✅ 文件传输
- ✅ 按组群发

### Sprint 4: 高级功能与跨平台 (2周)
- ✅ 截图功能
- ✅ 表情包
- ✅ RK3566移植

### Sprint 5: NPU加速(NSFW) (2周)
- 🔄 AI引擎架构
- 🔄 NPU集成
- 🔄 NSFW双端检测

### Sprint 6: 完整AI功能 (2周)
- ⏳ GPU加速
- ⏳ 语义搜索
- ⏳ 消息翻译
- ⏳ 聊天摘要

## 📖 学习资源

### 官方文档
- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [ONNX Runtime](https://onnxruntime.ai/docs/)
- [RKNPU Toolkit](https://github.com/rockchip-linux/rknn-toolkit2)
- [Protocol Buffers](https://developers.google.com/protocol-buffers)

### 推荐书籍
- 《Effective Modern C++》 - C++最佳实践
- 《Qt 6 C++ GUI Programming》 - Qt开发
- 《Scrum Guide》 - 敏捷开发

## 🤝 贡献指南

本项目采用多Agent协作开发模式：

1. **需求提出**：通过Requirements Analyst Agent分析
2. **方案设计**：通过Architecture Design Agent设计
3. **代码开发**：通过Development Execution Agent实现
4. **代码审查**：通过Code Review Agent审查
5. **测试验证**：通过Testing Agent测试
6. **质量监控**：通过Quality Assurance Agent监控

详细流程见 [多Agent项目迭代工作流.md](./多Agent项目迭代工作流.md)。

## 📝 版本历史

- **v1.0.0** (计划中) - 第一个完整版本
  - 核心P2P通信
  - 群聊和文件传输
  - NSFW检测(NPU加速)
  
- **v1.1.0** (计划中) - AI增强版本
  - GPU加速
  - 语义搜索
  - 消息翻译
  - 聊天摘要

## 📄 许可证

本项目文档采用 CC BY-NC-SA 4.0 许可证。

代码部分(待开发)将采用 MIT 许可证。

## 📧 联系方式

如有问题或建议，请：
- 查阅文档：优先阅读4个核心文档
- 提交Issue：描述问题和复现步骤
- 参与讨论：分享经验和最佳实践

---

**开始你的智能飞秋开发之旅！** 🚀

建议阅读顺序：
1. 本README (5分钟)
2. 飞秋方案.md (30分钟)
3. 多Agent项目迭代工作流.md (20分钟)
4. 项目总结与Agent使用指南.md (15分钟)
5. agent_prompts.yaml (根据需要查阅)
