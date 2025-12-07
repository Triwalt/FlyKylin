# FlyKylin 快速开始指南

本指南帮助您在Windows上快速搭建FlyKylin开发环境。

## 环境要求

### 必需工具

- **Windows 10/11** (64-bit)
- **Visual Studio 2022** - C++ Desktop Development workload
- **Qt 6.5+** - MSVC 2022 64-bit版本
- **CMake 3.20+**
- **Git**

### 可选工具

- **ONNX Runtime 1.23.2** - AI功能需要（已包含在3rdparty目录）
- **Protobuf** - 通过vcpkg自动安装

## 快速构建（推荐）

### 方式1：使用CMake Preset

```powershell
# 1. 克隆项目
git clone https://github.com/user/FlyKylin.git
cd FlyKylin

# 2. 初始化vcpkg子模块
git submodule update --init --recursive

# 3. 配置和构建（一步完成）
cmake --preset windows-release
cmake --build build/windows-release --config Release

# 4. 运行
.\build\windows-release\bin\FlyKylin.exe
```

### 方式2：传统CMake构建

```powershell
# 在VS Developer Command Prompt中执行
cd FlyKylin

# 配置
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake

# 构建
cmake --build build --config Release

# 运行
.\build\bin\Release\FlyKylin.exe
```

## 环境检查

运行环境检查脚本验证所有依赖：

```powershell
.\scripts\check-environment.ps1
```

预期输出：
```
[OK] Visual Studio 2022
[OK] Qt 6.x MSVC 2022 64-bit
[OK] CMake 3.20+
[OK] ONNX Runtime C++ API
[OK] Protobuf
```

## 项目结构

```
FlyKylin/
├── src/
│   ├── core/              # 核心业务逻辑
│   │   ├── communication/ # P2P通信
│   │   ├── ai/           # AI引擎
│   │   ├── services/     # 业务服务
│   │   └── database/     # 数据持久化
│   ├── ui/
│   │   ├── qml/          # QML界面
│   │   └── viewmodels/   # 视图模型
│   └── platform/         # 平台特定代码
├── model/
│   ├── onnx/             # ONNX模型
│   └── rknn/             # RKNN模型
├── protocol/             # Protobuf定义
├── tests/                # 单元测试
├── scripts/              # 构建脚本
├── 3rdparty/            # 第三方库
└── docs/                 # 文档
```

## 常见问题

### Q: MSVC编译器未找到

**解决方案**: 使用VS Developer Command Prompt
- 开始菜单 → "Developer Command Prompt for VS 2022"
- 或运行: `.\tools\developer\start-vsdevcmd.ps1`

### Q: Qt未找到

**解决方案**: 设置Qt路径
```powershell
$env:Qt6_DIR = "C:\Qt\6.5.3\msvc2019_64"
```

### Q: vcpkg依赖安装失败

**解决方案**: 手动安装
```powershell
.\vcpkg\vcpkg install protobuf:x64-windows
.\vcpkg\vcpkg install abseil:x64-windows
```

### Q: ONNX Runtime未找到

ONNX Runtime已包含在`3rdparty/onnxruntime`目录。如需更新：
1. 从 https://github.com/microsoft/onnxruntime/releases 下载
2. 解压到 `3rdparty/onnxruntime`

## 下一步

- [部署指南](DEPLOYMENT_GUIDE.md) - Windows和RK3566部署
- [需求文档](requirements/FlyKylin需求.md) - 项目需求
- [RKNN调试](RKNN_DEBUG_SUMMARY.md) - RK3566 NPU调试

## 开发工具

| 脚本 | 用途 |
|------|------|
| `scripts/build-windows.ps1` | Windows构建 |
| `scripts/check-environment.ps1` | 环境检查 |
| `tools/developer/verify-environment.ps1` | 详细环境验证 |

---

**遇到问题？** 查看 [常见问题](#常见问题) 或提交Issue。
