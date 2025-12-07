# Design Document: FlyKylin Documentation Update

## Overview

本设计文档描述了FlyKylin项目文档全面更新的方案。主要目标是将README.md和相关文档与项目的实际实现状态同步，移除过时信息，并提供准确的构建和部署指南。

## Architecture

文档更新涉及以下文件：

```
FlyKylin/
├── README.md                    # 主文档 - 需要全面重写
├── ARCHITECTURE_PROPOSAL.md     # 架构提案 - 需要更新状态说明
├── docs/
│   ├── GETTING_STARTED.md       # 快速开始 - 需要更新
│   ├── DEPLOYMENT_GUIDE.md      # 部署指南 - 已较新，小幅更新
│   └── requirements/
│       └── FlyKylin需求.md      # 需求文档 - 需要更新Sprint状态
└── scripts/
    └── README.md                # 脚本说明 - 需要验证准确性
```

## Components and Interfaces

### 1. README.md 重构

**当前问题：**
- 声称UI为Qt Widgets，实际已使用QML
- 引用不存在的文件（飞秋方案.md等）
- Sprint状态过时
- 技术栈信息不准确

**更新内容：**
- 项目简介和定位
- 实际实现的功能列表
- 准确的技术栈
- 正确的项目结构
- 更新的构建指南
- 准确的Sprint状态

### 2. 技术栈更新

**实际技术栈：**
| 组件 | 技术 | 版本 |
|------|------|------|
| 语言 | C++ | C++17 |
| UI框架 | Qt QML | Qt6 (Windows) / Qt5.12 (RK3566) |
| 网络 | Qt Network + Protobuf | - |
| AI推理 | ONNX Runtime | 1.23.2 |
| NPU推理 | RKNN | 2.3.2 (RK3566) |
| 数据库 | SQLite | - |
| 协程 | QCoro | 0.10.0 |
| 构建 | CMake + vcpkg | CMake 3.20+ |

### 3. 项目结构更新

**实际结构：**
```
FlyKylin/
├── src/
│   ├── core/
│   │   ├── communication/    # P2P通信 (UDP发现, TCP连接)
│   │   ├── ai/              # AI引擎 (NSFW检测, 文本嵌入)
│   │   ├── services/        # 业务服务 (消息, 文件传输, 群聊)
│   │   ├── database/        # 数据持久化
│   │   ├── config/          # 配置管理
│   │   ├── adapters/        # 适配器 (Protobuf序列化)
│   │   ├── models/          # 数据模型
│   │   └── ports/           # 接口定义
│   ├── ui/
│   │   ├── qml/             # QML界面文件
│   │   ├── viewmodels/      # MVVM视图模型
│   │   ├── widgets/         # Qt Widgets (备用)
│   │   └── windows/         # 窗口类
│   └── platform/            # 平台特定代码
├── model/
│   ├── onnx/                # ONNX模型文件
│   └── rknn/                # RKNN模型文件
├── protocol/                # Protobuf定义
├── tests/                   # 单元测试
├── scripts/                 # 构建和部署脚本
├── 3rdparty/               # 第三方库
└── docs/                    # 文档
```

## Data Models

文档更新不涉及数据模型变更。

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

由于本需求是文档更新，所有验收标准都是具体示例而非通用属性。文档更新的正确性通过以下方式验证：

1. **文件存在性检查**: 所有引用的文件必须存在
2. **内容一致性检查**: 文档描述必须与代码实现一致
3. **构建命令验证**: 文档中的构建命令必须可执行

由于这些都是具体的示例测试而非属性测试，本设计不包含Property-Based Testing的正式属性定义。

## Error Handling

文档更新过程中的错误处理：

1. **引用检查**: 在更新文档时，验证所有文件引用的有效性
2. **版本一致性**: 确保文档中的版本号与实际配置文件一致
3. **路径验证**: 确保所有路径引用正确

## Testing Strategy

### 验证方法

由于这是文档更新任务，测试策略侧重于手动验证和自动化检查：

1. **文件存在性验证**
   - 检查README.md中引用的所有文件是否存在
   - 验证项目结构描述与实际目录匹配

2. **构建命令验证**
   - 执行文档中的CMake preset命令
   - 验证构建成功

3. **内容准确性审查**
   - 对比文档描述与代码实现
   - 验证技术栈版本信息

### 验收检查清单

- [ ] README.md中无引用不存在的文件
- [ ] 项目结构与实际目录匹配
- [ ] 技术栈版本与CMakeLists.txt一致
- [ ] Sprint状态反映实际完成情况
- [ ] 构建命令可正常执行

