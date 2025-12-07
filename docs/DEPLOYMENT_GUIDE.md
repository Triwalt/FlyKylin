# FlyKylin 部署指南

## 目录

1. [平台概述](#平台概述)
2. [Windows 部署 (Qt6)](#windows-部署-qt6)
3. [RK3566 板端部署 (Qt5)](#rk3566-板端部署-qt5)
4. [Qt5/Qt6 兼容性说明](#qt5qt6-兼容性说明)
5. [脚本使用指南](#脚本使用指南)
6. [常见问题](#常见问题)

---

## 平台概述

| 平台 | Qt版本 | 构建工具 | AI推理 | 备注 |
|------|--------|----------|--------|------|
| Windows x64 | Qt6 | vcpkg + CMake | ONNX Runtime | 主开发平台 |
| RK3566 ARM64 | Qt5.12.8 | 交叉编译 | RKNN (NPU) | 嵌入式目标平台 |
| Linux x64 | Qt5/Qt6 | 系统包 | ONNX Runtime | CI/测试环境 |

### 关键配置差异

```
CMakeLists.txt 配置:
├── FLYKYLIN_FORCE_QT5=ON    → 强制使用Qt5
├── FLYKYLIN_FORCE_RK3566=ON → 启用RK3566平台特性
└── IS_RK3566                → 自动检测ARM64架构
```

---

## Windows 部署 (Qt6)

### 1. 环境要求

- Windows 10/11 x64
- Visual Studio 2022 (MSVC)
- CMake 3.20+
- Qt6 (6.5+ 推荐)
- vcpkg

### 2. 依赖安装

```powershell
# 安装 vcpkg 依赖
vcpkg install protobuf:x64-windows
vcpkg install abseil:x64-windows
vcpkg install utf8-range:x64-windows

# 设置环境变量
$env:Qt6_DIR = "C:\Qt\6.5.3\msvc2019_64"
$env:VCPKG_ROOT = "C:\vcpkg"
```

### 3. 构建命令

```powershell
# 配置 (使用 CMake Presets)
cmake --preset windows-x64-release

# 构建
cmake --build build/windows-x64-release --config Release

# 或使用 Visual Studio
cmake -G "Visual Studio 17 2022" -A x64 -B build
cmake --build build --config Release
```

### 4. 运行

```powershell
# 直接运行
.\build\bin\Release\FlyKylin.exe

# 或使用 windeployqt 打包
windeployqt --qmldir src\ui\qml build\bin\Release\FlyKylin.exe
```

### 5. 环境检查

```powershell
# 运行环境检查脚本
.\scripts\check-environment.ps1
```

---

## RK3566 板端部署 (Qt5)

### 1. 环境信息

- 板端IP: `192.168.100.2`
- 用户: `kylin`
- 密码: `123456`
- 项目目录: `/home/kylin/FlyKylinApp`
- Qt版本: Qt5.12.8 (系统自带)
- RKNN版本: 2.3.2

### 2. 交叉编译环境 (WSL)

```bash
# 安装交叉编译工具链
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# 设置 sysroot (从板端同步)
export SYSROOT=/opt/rk3566-sysroot

# 配置交叉编译
cmake --preset linux-arm64-rk3566-cross \
    -DCMAKE_SYSROOT=$SYSROOT \
    -DFLYKYLIN_FORCE_QT5=ON \
    -DFLYKYLIN_FORCE_RK3566=ON
```

### 3. 构建命令

```bash
# 在 WSL 中构建
wsl -d Ubuntu1 -- bash -c "cd /mnt/e/Project/FlyKylin && \
    cmake --build build/linux-arm64-rk3566-cross --target FlyKylin -j8"
```

### 4. 部署到板端

```bash
# 部署可执行文件
wsl -d Ubuntu1 -- sshpass -p 123456 scp \
    /mnt/e/Project/FlyKylin/build/linux-arm64-rk3566-cross/bin/FlyKylin \
    kylin@192.168.100.2:/home/kylin/FlyKylinApp/bin/

# 部署 RKNN 模型
wsl -d Ubuntu1 -- sshpass -p 123456 scp \
    /mnt/e/Project/FlyKylin/model/rknn/open_nsfw_rk3566.rknn \
    kylin@192.168.100.2:/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn

# 部署 QML 文件 (如有更新)
wsl -d Ubuntu1 -- sshpass -p 123456 scp -r \
    /mnt/e/Project/FlyKylin/src/ui/qml \
    kylin@192.168.100.2:/home/kylin/FlyKylinApp/share/
```

### 5. 板端启动

```bash
# 基本启动
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 \
    "cd /home/kylin/FlyKylinApp && ./run-flykylin.sh"

# 优化启动 (推荐，针对低内存环境)
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 \
    "cd /home/kylin/FlyKylinApp && ./run-flykylin-optimized.sh"

# 后台启动
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 \
    "cd /home/kylin/FlyKylinApp && nohup ./run-flykylin.sh > /tmp/flykylin.log 2>&1 &"
```

### 6. 查看日志

```bash
# 实时查看日志
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 \
    "tail -f /tmp/flykylin.log"

# 查看最近50行
wsl -d Ubuntu1 -- sshpass -p 123456 ssh kylin@192.168.100.2 \
    "tail -50 /tmp/flykylin.log"
```

### 7. 板端目录结构

```
/home/kylin/FlyKylinApp/
├── bin/
│   ├── FlyKylin              # 主程序
│   └── models/
│       └── open_nsfw.rknn    # RKNN 模型
├── lib/                      # 依赖库
├── share/
│   └── qml/                  # QML 文件
├── run-flykylin.sh           # 基本启动脚本
└── run-flykylin-optimized.sh # 优化启动脚本
```

---

## Qt5/Qt6 兼容性说明

### QML 版本兼容

项目 QML 文件使用 Qt5.12 兼容的 import 语句：

```qml
// 兼容 Qt5.12+ 和 Qt6
import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
```

### 主要差异

| 特性 | Qt5 | Qt6 | 处理方式 |
|------|-----|-----|----------|
| QtConcurrent | ✅ | ✅ | RK3566上不可用，CMake条件排除 |
| Qt.labs.settings | ✅ | ✅ | 两版本兼容 |
| QML import | 2.x | 6.x | 使用2.x语法，Qt6向后兼容 |
| C++ API | 部分差异 | 新API | 使用条件编译 |

### CMake 配置

```cmake
# Qt版本检测逻辑
option(FLYKYLIN_FORCE_QT5 "Prefer Qt5 over Qt6" OFF)

if(FLYKYLIN_FORCE_QT5)
    find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS ...)
else()
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ...)
endif()

# RK3566 上排除 QtConcurrent
if(NOT IS_RK3566)
    list(APPEND _FLYKYLIN_QT_COMPONENTS Concurrent)
endif()
```

### 平台特定 QML 配置

`src/ui/qml/PlatformConfig.qml` 提供平台自适应配置：

```qml
pragma Singleton
QtObject {
    readonly property bool isRK3566: ...
    
    // 图片配置 - RK3566 使用更小的尺寸
    readonly property int imageSourceWidth: isRK3566 ? 260 : 1024
    readonly property int imageDisplayMaxWidth: isRK3566 ? 200 : 260
    
    // 动画配置 - RK3566 简化动画
    readonly property bool enableAnimations: !isRK3566
}
```

---

## 脚本使用指南

### 脚本列表

| 脚本 | 平台 | 用途 |
|------|------|------|
| `build-windows.ps1` | Windows | Windows 构建和打包 |
| `build-rk3566-cross.sh` | WSL/Linux | RK3566 交叉编译 |
| `deploy-to-board.sh` | WSL/Linux | 部署到 RK3566 板端 |
| `run-flykylin.sh` | Linux/RK3566 | 基本启动脚本 |
| `run-flykylin-optimized.sh` | RK3566 | 优化启动脚本（推荐） |
| `check-environment.sh` | Linux | 环境检查 |
| `check-environment.ps1` | Windows | 环境检查 |

详细脚本说明请参考 [scripts/README.md](../scripts/README.md)

### build-windows.ps1

Windows 构建脚本，支持配置、构建、清理和打包：

```powershell
# 完整构建流程
.\scripts\build-windows.ps1 all

# 仅构建
.\scripts\build-windows.ps1 build -Config Release

# 清理
.\scripts\build-windows.ps1 clean
```

### build-rk3566-cross.sh

RK3566 交叉编译脚本：

```bash
# 完整编译
./scripts/build-rk3566-cross.sh --all

# 指定 sysroot
SYSROOT=/opt/rk3566-sysroot ./scripts/build-rk3566-cross.sh --all
```

### deploy-to-board.sh

一键部署脚本，支持部署可执行文件、模型、QML 和脚本：

```bash
# 部署所有内容并启动
./scripts/deploy-to-board.sh --all

# 仅部署可执行文件并重启
./scripts/deploy-to-board.sh --binary

# 查看日志
./scripts/deploy-to-board.sh --log
```

### run-flykylin.sh

基本启动脚本，设置必要的环境变量：

```bash
export DISPLAY=:0
export LD_LIBRARY_PATH="${APP_ROOT}/lib:..."
export QT_PLUGIN_PATH="/usr/lib/aarch64-linux-gnu/qt5/plugins"
export QT_QPA_PLATFORM=xcb
```

### run-flykylin-optimized.sh

针对 RK3566 低内存环境的优化启动脚本：

1. **内存优化**: 关闭不必要的后台服务（蓝牙、侧边栏等）
2. **NPU 唤醒**: 检查并唤醒 NPU 设备
3. **RKNN 检查**: 验证 RKNN 库和模型
4. **Qt 优化**: 使用软件渲染减少 GPU 内存占用
5. **进程管理**: 杀死旧实例，使用 nice 降低优先级

关键环境变量：
```bash
export QT_QUICK_BACKEND=software    # 软件渲染
export QSG_RENDER_LOOP=basic        # 基本渲染循环
export MALLOC_TRIM_THRESHOLD_=65536 # 积极释放内存
```

---

## 常见问题

### Q1: 板端启动后黑屏

检查 DISPLAY 环境变量和 X11 权限：
```bash
export DISPLAY=:0
xhost +local:
```

### Q2: RKNN 模型加载失败

1. 检查模型路径: `/home/kylin/FlyKylinApp/bin/models/open_nsfw.rknn`
2. 检查 RKNN 库: `ls -la /lib/librknnrt.so`
3. 检查 NPU 状态: `cat /sys/devices/platform/fde40000.npu/power/runtime_status`

### Q3: 内存不足导致崩溃

使用优化启动脚本：
```bash
./run-flykylin-optimized.sh
```

### Q4: Qt 插件找不到

确保 QT_PLUGIN_PATH 正确设置：
```bash
export QT_PLUGIN_PATH="/usr/lib/aarch64-linux-gnu/qt5/plugins"
```

### Q5: 交叉编译链接错误

检查 sysroot 中的 Qt5 库路径：
```bash
ls -la $SYSROOT/usr/lib/aarch64-linux-gnu/libQt5*.so
```

---

## 备份与恢复

### 创建备份

```bash
# 板端备份到 SD 卡
ssh kylin@192.168.100.2 "cd /home/kylin && \
    tar -czvf /media/kylin/EED4-7516/FlyKylinApp_backup_$(date +%Y%m%d_%H%M%S).tar.gz FlyKylinApp"
```

### 恢复备份

```bash
# 从 SD 卡恢复
ssh kylin@192.168.100.2 "cd /home/kylin && \
    tar -xzvf /media/kylin/EED4-7516/FlyKylinApp_backup_YYYYMMDD_HHMMSS.tar.gz"
```

---

## 相关文档

- [RKNN 调试总结](./RKNN_DEBUG_SUMMARY.md) - RKNN NSFW 检测调试记录
- [环境安装指南](./环境安装指南.md) - 详细的环境配置步骤
- [用户测试指南](./用户测试指南.md) - 功能测试说明
