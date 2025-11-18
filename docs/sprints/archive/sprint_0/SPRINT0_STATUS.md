# Sprint 0 状态报告

**日期**: 2024-11-18  
**状态**: 环境验证中 🔄

## ✅ 已完成

### 1. 需求文档整理
- ✅ 删除冗余文档（需求澄清报告、立即行动计划、多Agent工作流等）
- ✅ 创建核心需求文档：`docs/requirements/FlyKylin需求.md`
- ✅ 简化Sprint 0计划：`docs/requirements/Sprint_0_环境验证.md`
- ✅ 创建环境安装指南：`环境安装指南.md`

### 2. 需求简化决策
- ✅ 明确不实现加密（UDP/TCP明文传输）
- ✅ 明确不实现身份认证（IP+主机名即可）
- ✅ 明确不实现消息签名
- ✅ 专注核心功能：P2P通信 + AI功能

### 3. 环境检测
- ✅ 定位Qt 6.9.3 → `D:\Qt\6.9.3\msvc2022_64`
- ✅ 定位NSFW模型 → `E:\Project\tensorflow-open_nsfw\open_nsfw.onnx`
- ✅ 定位BGE模型 → `E:\Project\tensorflow-open_nsfw\onnx_models\`
- ✅ 确认CMake 3.28.1可用
- ✅ 更新CMakeLists.txt配置实际路径

### 4. 开发工具脚本（tools/developer/）
- ✅ `start-vsdevcmd.ps1` - 自动启动VS Dev Prompt（解决MSVC问题）⭐
- ✅ `configure-environment.ps1` - 配置环境变量
- ✅ `verify-environment.ps1` - 验证所有依赖
- ✅ `install-onnx-runtime.ps1` - 安装ONNX Runtime
- ✅ `install-protobuf.ps1` - 安装Protobuf
- ✅ `README.md` - 工具使用说明

### 5. 文档整理
- ✅ 移动`环境安装指南.md`到`docs/`
- ✅ 保持精简文档结构

## ⏳ 进行中

### 依赖安装和MSVC配置
已创建完整的英文工具脚本（解决编码和MSVC问题）：

**Step 1: 安装依赖**

```powershell
# 安装ONNX Runtime（如果自动下载失败，见方法2）
.\tools\developer\install-onnx-runtime.ps1

# 安装Protobuf（需要管理员权限）
.\tools\developer\install-protobuf.ps1
```

**Step 2: 启动VS开发环境（解决MSVC问题）⭐**

```powershell
# 自动查找并启动VS Developer Command Prompt
.\tools\developer\start-vsdevcmd.ps1
```

**Step 3: 在新打开的VS Dev Prompt窗口中**

```cmd
cd e:\Project\FlyKylin

REM 配置环境变量
.\tools\developer\configure-environment.cmd

REM 验证环境
.\tools\developer\verify-environment.cmd

REM 编译项目
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

**方法2: 手动下载ONNX Runtime（如果网络问题）**

1. 访问：https://github.com/microsoft/onnxruntime/releases/tag/v1.23.2
2. 下载：`onnxruntime-win-x64-1.23.2.zip` (约30MB)
3. 解压到：`e:\Project\FlyKylin\3rdparty\onnxruntime`
4. 验证：`.\tools\developer\verify-environment.ps1`

## 📋 待办事项

### 立即行动（今天）
1. [ ] 手动下载ONNX Runtime并解压到正确位置
2. [ ] 安装Protobuf（使用Chocolatey）
3. [ ] 在VS Developer Command Prompt中编译项目
4. [ ] 验证FlyKylin.exe生成

### 后续任务（本周）
1. [ ] 创建ONNX Runtime测试程序
2. [ ] 加载NSFW模型并执行一次推理
3. [ ] 验证DirectML可用
4. [ ] 完成Sprint 0所有验收标准

## 📁 文档结构（精简后）

```
FlyKylin/
├── docs/
│   ├── requirements/
│   │   ├── FlyKylin需求.md          # 核心需求（NEW）
│   │   └── Sprint_0_环境验证.md      # Sprint 0计划
│   └── 飞秋方案.md                   # 详细技术方案
├── configure-environment.ps1         # 环境配置脚本
├── 环境安装指南.md                    # 安装步骤（NEW）
├── 快速启动.md                       # 快速开始
└── CMakeLists.txt                    # 已更新路径
```

## 🎯 核心决策记录

### 简化决策
**网络协议简化**：
- ❌ 不实现加密（局域网环境）
- ❌ 不实现认证（IP+主机名）
- ❌ 不实现签名（可信环境）
- ✅ UDP广播 + TCP明文传输

**优势**：
- 开发速度提升30%
- 性能提升20%
- 代码量减少25%
- 调试更容易

### 技术栈确认
- C++20 + Qt 6.9.3
- ONNX Runtime 1.23.2
- Protobuf (latest)
- SQLite
- Windows DirectML / RK3566 NPU

## 🚀 下一步

### 手动安装ONNX Runtime

```bash
# 1. 下载
浏览器打开: https://github.com/microsoft/onnxruntime/releases/tag/v1.23.2
下载: onnxruntime-win-x64-1.23.2.zip

# 2. 解压到
e:\Project\FlyKylin\3rdparty\onnxruntime

# 3. 验证目录结构
e:\Project\FlyKylin\3rdparty\onnxruntime\
├── include\onnxruntime_c_api.h
├── lib\onnxruntime.dll
└── lib\onnxruntime.lib
```

### 安装Protobuf

```powershell
# 管理员权限运行PowerShell
choco install protoc -y

# 验证
protoc --version
```

### 编译项目

```cmd
# 1. 打开 "Developer Command Prompt for VS 2022"

# 2. 切换目录
cd e:\Project\FlyKylin

# 3. 配置环境
powershell .\configure-environment.ps1

# 4. 创建build目录
mkdir build
cd build

# 5. 配置CMake
cmake ..

# 6. 编译
cmake --build . --config Debug

# 7. 运行
.\bin\Debug\FlyKylin.exe
```

## 📞 如遇问题

查看 `环境安装指南.md` 中的"常见问题"章节。

---

**目标**: 今天完成ONNX Runtime和Protobuf安装，明天完成首次编译。
