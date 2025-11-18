# FlyKylin 项目需求

**版本**: 1.0  
**日期**: 2024-11-18  
**状态**: Sprint 0 进行中

## 📋 项目定位

局域网P2P即时通讯工具，集成AI功能（NSFW检测、语义搜索），支持Windows和RK3566跨平台。

## 🎯 核心原则

### 简化决策（重要）

**网络协议**：
- ✅ UDP广播节点发现（局域网可靠）
- ✅ TCP明文消息传输
- ❌ **不实现加密**（局域网环境，性能优先）
- ❌ **不实现身份认证**（IP+主机名即可）
- ❌ **不实现消息签名**（可信环境）

**理由**：
- 局域网部署，网络相对可信
- 开发速度提升30%，性能提升20%
- 代码量减少25%，调试更容易

## 🚀 核心功能

### P2P通信
- UDP广播节点自动发现（5秒间隔）
- TCP长连接消息传输
- Protobuf明文序列化
- 文件传输（独立TCP端口）

### 用户体系
- 使用IP地址+主机名作为用户标识
- 本地配置用户名和头像
- 用户分组管理
- 在线/离线状态

### 群聊功能
- P2P多发实现群聊
- 分组群发
- 消息历史记录（SQLite）

### AI功能
- **NSFW检测**（双端检测）
  - 模型：MobileNetV3 (open_nsfw.onnx)
  - Windows：DirectML加速
  - RK3566：NPU加速
  - 接收方检测+发送方预检测

- **语义搜索**
  - 模型：BAAI/bge-small-zh-v1.5
  - 聊天记录向量化
  - 语义相似度搜索

- **消息翻译**（可选，Windows）
  - 中英互译
  - 仅在Windows上实现

## 🏗️ 技术架构

### 技术栈
- **语言**: C++20
- **UI框架**: Qt 6.9.3
- **AI引擎**: ONNX Runtime 1.23.2
- **通信**: UDP + TCP + Protobuf
- **数据库**: SQLite
- **平台**: Windows (MSVC 2022), RK3566 (GCC 11+)

### 架构模式
- **三层架构**: UI层 → 业务逻辑层 → 数据访问层
- **MVVM**: UI层使用ViewModel
- **HAL**: AI加速器硬件抽象层

### 关键组件
```
FlyKylin/
├── src/
│   ├── core/              # 核心业务逻辑
│   │   ├── communication/ # P2P通信服务
│   │   ├── ai/           # AI引擎
│   │   └── storage/      # 数据持久化
│   ├── platform/         # 平台抽象
│   │   ├── windows/      # DirectML实现
│   │   └── rk3566/       # NPU实现
│   └── ui/               # Qt界面
├── 3rdparty/             # 第三方库
│   ├── onnxruntime/      # 需下载
│   └── protobuf/         # 需安装
└── models/               # AI模型（链接到外部）
```

## 📊 Sprint规划

### Sprint 0: 环境验证（1周）⏳

**目标**: 验证Windows开发环境，完成首次编译

**任务**:
- [x] 定位Qt 6.9.3和ONNX模型
- [ ] 下载ONNX Runtime C++ API
- [ ] 安装Protobuf
- [ ] 编译Hello World Qt程序
- [ ] 测试ONNX Runtime C++ API

**DoD**:
- Windows环境编译成功
- ONNX Runtime能加载NSFW模型
- 输出FlyKylin.exe可执行文件

### Sprint 1: P2P通信基础（2周）

**核心功能**:
- UDP广播节点发现
- TCP长连接
- Protobuf消息协议
- 基础UI（用户列表）

**不实现**:
- ❌ 加密
- ❌ 认证
- ❌ mDNS

### Sprint 2: 用户与分组（2周）

**核心功能**:
- 用户信息管理（本地配置）
- 分组管理
- 好友列表UI
- 聊天窗口

### Sprint 3: 群聊与文件（2周）

**核心功能**:
- 群聊（P2P多发）
- 文件传输（TCP）
- 接收确认
- 文件管理UI

### Sprint 4: 跨平台（2周）

**核心功能**:
- RK3566首次编译
- 平台特定UI适配
- 截图功能
- 表情包

### Sprint 5: AI-NSFW检测（2周）

**核心功能**:
- AIEngine抽象层
- Windows DirectML实现
- RK3566 NPU实现
- 双端NSFW检测

### Sprint 6: AI-语义搜索（2周）

**核心功能**:
- BGE模型集成
- 消息向量化
- 语义搜索API
- 搜索UI

## 🎯 当前环境状态

### ✅ 已就绪
- Qt 6.9.3 MSVC 2022 64-bit → `D:\Qt\6.9.3\msvc2022_64`
- NSFW模型 → `E:\Project\tensorflow-open_nsfw\open_nsfw.onnx`
- BGE模型 → `E:\Project\tensorflow-open_nsfw\onnx_models\bge-small-zh-v1.5\`
- CMake 3.28.1

### ⚠️ 待安装
- ONNX Runtime C++ API (1.23.2)
- Protobuf compiler

## 📐 Protobuf消息定义（简化版）

```protobuf
syntax = "proto3";

message Message {
    string sender_id = 1;      // IP地址
    string sender_name = 2;    // 用户名
    string host_name = 3;      // 主机名
    MessageType type = 4;      // 消息类型
    bytes payload = 5;         // 明文负载
    uint64 timestamp = 6;      // 时间戳
}

enum MessageType {
    DISCOVERY = 0;    // 节点发现
    TEXT = 1;         // 文本消息
    FILE = 2;         // 文件传输
    HEARTBEAT = 3;    // 心跳
}
```

## ⚡ 性能目标

### Windows
- 启动时间 < 2秒
- UI响应 < 100ms
- P2P发现延迟 < 5秒
- NSFW检测 < 500ms

### RK3566
- 启动时间 < 5秒
- UI响应 < 200ms
- NSFW检测 < 1秒
- 内存占用 < 200MB

## 📝 Definition of Done

每个Sprint完成标准：

- [ ] 功能实现所有验收标准
- [ ] 代码通过Code Review
- [ ] 单元测试覆盖率 ≥ 70%
- [ ] Windows平台验证通过
- [ ] RK3566平台验证通过（Sprint 4+）
- [ ] 性能达标
- [ ] 文档更新

## 🚫 不实现的功能

明确**不在范围内**的功能：

- ❌ 端到端加密
- ❌ 消息数字签名
- ❌ 用户注册/登录系统
- ❌ 中心服务器
- ❌ 公网部署支持
- ❌ 离线消息同步
- ❌ 视频通话
- ❌ 语音消息
- ❌ 断点续传（MVP阶段）

## 📚 参考资源

- [飞秋方案](../飞秋方案.md) - 详细技术方案
- [快速启动](../../快速启动.md) - 环境配置指南
- [需求简化说明](./需求简化说明.md) - 简化决策理由

---

**简洁、快速、实用 - FlyKylin的核心价值** 🚀
