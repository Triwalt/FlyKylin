# FlyKylin 脚本目录

本目录包含 FlyKylin 项目的构建、部署和环境检查脚本。

## 脚本列表

| 脚本 | 平台 | 用途 |
|------|------|------|
| `build-windows.ps1` | Windows | Windows 构建和打包 |
| `build-rk3566-cross.sh` | WSL/Linux | RK3566 交叉编译 |
| `deploy-to-board.sh` | WSL/Linux | 部署到 RK3566 板端 |
| `run-flykylin.sh` | Linux/RK3566 | 基本启动脚本 |
| `run-flykylin-optimized.sh` | RK3566 | 优化启动脚本 |
| `check-environment.sh` | Linux | 环境检查 |
| `check-environment.ps1` | Windows | 环境检查 |

---

## Windows 构建

```powershell
# 配置 + 构建 + 打包
.\scripts\build-windows.ps1 all

# 仅构建
.\scripts\build-windows.ps1 build

# 构建 Debug 版本
.\scripts\build-windows.ps1 build -Config Debug

# 详细输出
.\scripts\build-windows.ps1 configure -DetailedOutput

# 清理
.\scripts\build-windows.ps1 clean
```

---

## RK3566 交叉编译

在 WSL 或 Linux 环境中运行：

```bash
# 配置 + 构建
./scripts/build-rk3566-cross.sh --all

# 仅构建
./scripts/build-rk3566-cross.sh --build

# 指定 sysroot
SYSROOT=/opt/rk3566-sysroot ./scripts/build-rk3566-cross.sh --all

# 清理
./scripts/build-rk3566-cross.sh --clean
```

---

## 板端部署

在 WSL 中运行：

```bash
# 部署所有内容并启动
./scripts/deploy-to-board.sh --all

# 仅部署可执行文件
./scripts/deploy-to-board.sh --binary

# 仅部署 RKNN 模型
./scripts/deploy-to-board.sh --model

# 重启应用
./scripts/deploy-to-board.sh --restart

# 查看日志
./scripts/deploy-to-board.sh --log

# 自定义板端地址
BOARD_IP=192.168.1.100 ./scripts/deploy-to-board.sh --all
```

---

## 板端启动

### 基本启动

```bash
./run-flykylin.sh
```

设置的环境变量：
- `DISPLAY=:0`
- `LD_LIBRARY_PATH` - 应用库路径
- `QT_PLUGIN_PATH` - Qt 插件路径
- `QT_QPA_PLATFORM=xcb`

### 优化启动 (推荐)

```bash
./run-flykylin-optimized.sh
```

额外功能：
- 关闭不必要的后台服务（蓝牙、侧边栏等）
- 清理内存缓存
- 检查并唤醒 NPU
- 验证 RKNN 库和模型
- 使用软件渲染减少 GPU 内存
- 使用 nice 降低进程优先级

---

## 环境检查

### Linux

```bash
./scripts/check-environment.sh
```

检查项目：
- GCC/G++/CMake/Make/Git
- Qt 框架
- Protobuf
- ONNX Runtime
- 代码质量工具
- Python 环境

### Windows

```powershell
.\scripts\check-environment.ps1
```

检查项目：
- MSVC 编译器
- CMake/Git
- Qt 框架
- vcpkg
- 代码质量工具
- Python 环境

---

## 环境变量

### 构建相关

| 变量 | 说明 | 默认值 |
|------|------|--------|
| `Qt6_DIR` | Qt6 安装路径 | 自动检测 |
| `VCPKG_ROOT` | vcpkg 根目录 | C:\vcpkg |
| `SYSROOT` | 交叉编译 sysroot | /opt/rk3566-sysroot |
| `CROSS_COMPILE` | 交叉编译前缀 | aarch64-linux-gnu- |
| `JOBS` | 并行编译数 | CPU 核心数 |

### 部署相关

| 变量 | 说明 | 默认值 |
|------|------|--------|
| `BOARD_IP` | 板端 IP 地址 | 192.168.100.2 |
| `BOARD_USER` | 板端用户名 | kylin |
| `BOARD_PASS` | 板端密码 | 123456 |
| `PROJECT_ROOT` | 项目根目录 | 自动检测 |

---

## 快速开始

### Windows 开发

```powershell
# 1. 检查环境
.\scripts\check-environment.ps1

# 2. 构建
.\scripts\build-windows.ps1 all

# 3. 运行
.\build\windows-x64-release\bin\Release\FlyKylin.exe
```

### RK3566 部署

```bash
# 1. 交叉编译 (WSL)
./scripts/build-rk3566-cross.sh --all

# 2. 部署到板端
./scripts/deploy-to-board.sh --all

# 3. 查看日志
./scripts/deploy-to-board.sh --log
```
