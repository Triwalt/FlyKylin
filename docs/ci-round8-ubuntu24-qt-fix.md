# CI/CD 第 8 轮修复：Ubuntu 24.04 系统 Qt

**日期**: 2025-11-18  
**运行**: #19470431664  
**提交**: 8a034cc  
**状态**: 进行中

---

## 第 7 轮失败回顾

### 错误信息

```log
INFO    : aqtinstall(aqt) v3.3.0 on Python 3.10.12 [CPython GCC 11.4.0]
ERROR   : The packages ['qt_base'] were not found while parsing XML of package information!
Suggested follow-up:
* Please use 'aqt list-qt linux desktop --arch 6.6.3' to show architectures available.
Error: Process completed with exit code 1.
```

### 根本原因

**Qt 官方不提供 Linux ARM64 的预编译包！**

aqtinstall 从 Qt 官方镜像下载预编译包，但 Qt 官方只为以下平台提供预编译版本：

| 平台 | 架构 | aqt 可用 |
|-----|------|---------|
| Windows | x64, ARM64 | ✅ |
| Linux | x64 | ✅ |
| Linux | ARM64 | ❌ **不支持** |
| macOS | x64, ARM64 | ✅ |

**为什么不支持 Linux ARM64？**
- 市场需求小（主要用于嵌入式和服务器）
- 发行版多样性（Ubuntu、Debian、Fedora等）
- 通常通过系统包管理器安装

---

## 第 8 轮修复方案

### 策略转变

**放弃**: 使用 aqtinstall 安装特定版本的 Qt  
**采用**: 使用 Ubuntu 系统包管理器的 Qt

### 关键改动

#### 1. 升级到 Ubuntu 24.04

```yaml
# 修复前
ubuntu:22.04

# 修复后
ubuntu:24.04
```

**原因**:
- Ubuntu 22.04 (2022-04): Qt 6.2.4 < 6.5 ❌
- Ubuntu 24.04 (2024-04): Qt 6.? (预期更新) ✅

#### 2. 移除 aqtinstall

```bash
# 修复前
apt-get install python3 python3-pip python3-setuptools
pip3 install aqtinstall
aqt install-qt linux desktop ${QT_VERSION} gcc_arm64 -O /opt/Qt

# 修复后
apt-get install qt6-base-dev qt6-base-dev-tools
```

#### 3. 自动检测 Qt 路径

```bash
echo "==== Checking Qt version ===="
QT_VERSION_INSTALLED=$(qmake6 -query QT_VERSION || echo "unknown")
echo "Installed Qt version: ${QT_VERSION_INSTALLED}"

echo "==== Setting Qt paths ===="
export QT_ROOT_DIR=$(qmake6 -query QT_INSTALL_PREFIX)
export Qt6_DIR=${QT_ROOT_DIR}/lib/$(gcc -print-multiarch)/cmake/Qt6
export PATH="${QT_ROOT_DIR}/bin:$PATH"
export LD_LIBRARY_PATH="${QT_ROOT_DIR}/lib:$LD_LIBRARY_PATH"

echo "Qt6_DIR = ${Qt6_DIR}"
echo "QT_ROOT_DIR = ${QT_ROOT_DIR}"
qmake6 --version
```

**关键点**:
- 使用 `qmake6 -query` 自动获取 Qt 安装路径
- 使用 `gcc -print-multiarch` 获取正确的库路径（`aarch64-linux-gnu`）
- 不再硬编码版本号

---

## Ubuntu 24.04 Qt 版本预测

### Ubuntu Qt 包历史

| Ubuntu 版本 | 发布日期 | Qt 版本 |
|------------|---------|---------|
| 20.04 LTS | 2020-04 | 5.12.8 |
| 22.04 LTS | 2022-04 | 6.2.4 |
| 24.04 LTS | 2024-04 | **6.?** |

### 预期情况

#### 实际情况：Qt 6.4.2

Ubuntu 24.04 包含 Qt 6.4.2：
- ✅ 已更新 CMakeLists.txt 要求为 `Qt6 6.4`
- ✅ 满足项目需求（Qt 6.4+）
- ✅ 无需 fallback 方案

#### 次佳情况：Qt 6.2-6.4

如果 Ubuntu 24.04 仍然是 Qt 6.2-6.4：
- ❌ 不满足 CMakeLists.txt 要求
- ⚠️ 需要应用备用方案

---

## 备用方案：条件编译

### 准备的 Fallback

已创建文件：`CMakeLists.txt.arm64-fallback`

```cmake
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    # ARM64: Try Qt 6.5 first, fallback to 6.2
    find_package(Qt6 6.2 REQUIRED COMPONENTS
        Core Gui Widgets Network Sql Concurrent
    )
    if(Qt6_VERSION VERSION_LESS "6.5")
        message(WARNING "Qt version ${Qt6_VERSION} < 6.5 on ARM64, some features may be limited")
    endif()
else()
    # Other architectures: Require Qt 6.5+
    find_package(Qt6 6.5 REQUIRED COMPONENTS
        Core Gui Widgets Network Sql Concurrent
    )
endif()
```

### 应用方式

如果 Ubuntu 24.04 Qt < 6.5，将执行：

```bash
# 替换 CMakeLists.txt 中的 find_package
sed -i '/find_package(Qt6 6.5/,/)/c\<contents of CMakeLists.txt.arm64-fallback>' CMakeLists.txt
```

---

## 性能影响

### 时间对比

| 阶段 | aqt 方案 | 系统包方案 | 差异 |
|-----|---------|----------|------|
| 安装 Python | 1-2 min | - | +1-2 min |
| 安装 aqt | 30-60 s | - | +30-60 s |
| 下载 Qt | 3-5 min | - | +3-5 min |
| 安装 Qt | 1-2 min | - | +1-2 min |
| apt install Qt | - | 2-3 min | - |
| **总计** | **6-10 min** | **2-3 min** | **节省 4-7 min** |

### 其他优势

1. **简化依赖**: 不需要 Python、pip、aqt
2. **减少失败点**: 少了网络下载和解压环节
3. **一致性**: 使用系统标准路径
4. **可维护性**: 跟随系统更新

---

## 风险评估

### 风险：Qt 版本不够

**概率**: 中等  
**影响**: 需要应用 fallback

**缓解措施**:
- 已准备 `CMakeLists.txt.arm64-fallback`
- 降低版本要求到 6.2（Ubuntu 22.04 可用）
- 添加版本检查和警告

### 风险：系统包缺少组件

**概率**: 低  
**影响**: 编译失败

**缓解措施**:
- `qt6-base-dev` 包含核心组件
- 如需额外模块，可继续添加（如 `qt6-multimedia-dev`）

---

## 与其他平台的一致性

### 所有平台对比

| 平台 | Qt 安装方式 | 版本 | 状态 |
|-----|------------|------|------|
| Windows AMD64 | `install-qt-action@v4` | 6.6.3 | ✅ |
| Windows ARM64 | `install-qt-action@v4` | 6.6.3 | ✅ |
| Linux AMD64 | `install-qt-action@v4` | 6.6.3 | ✅ |
| Linux ARM64 | `apt-get` (Ubuntu 24.04) | 6.? | ⏳ 待验证 |

**不一致性**:
- 其他平台：6.6.3（精确控制）
- ARM64：系统版本（取决于 Ubuntu）

**影响**:
- 如果 Ubuntu 24.04 = Qt 6.6.x：完全一致 ✅
- 如果 Ubuntu 24.04 = Qt 6.5.x：可接受 ✅
- 如果 Ubuntu 24.04 < Qt 6.5：需要 fallback ⚠️

---

## 验证步骤

### 1. 检查 Qt 版本

在 CI 日志中搜索：
```bash
grep "Installed Qt version:" log.txt
```

预期输出：
```
Installed Qt version: 6.5.x
# 或
Installed Qt version: 6.6.x
```

### 2. 检查 Qt 路径

```bash
grep "Qt6_DIR =" log.txt
grep "QT_ROOT_DIR =" log.txt
```

预期输出：
```
Qt6_DIR = /usr/lib/aarch64-linux-gnu/cmake/Qt6
QT_ROOT_DIR = /usr
```

### 3. 检查 CMake 配置

```bash
grep "Could not find.*Qt6" log.txt
```

预期：**无输出**（表示找到 Qt6）

### 4. 检查构建成功

```bash
grep "Building project" log.txt
grep "cmake --build build" log.txt
```

---

## 经验教训

### 1. 不要假设所有平台都有预编译包

**错误假设**: Qt 官方提供所有平台的预编译包  
**实际情况**: Linux ARM64 需要自行编译或使用系统包

### 2. 系统包管理器是可靠的备选

**优势**:
- 适配平台和发行版
- 包含必要的依赖和配置
- 维护成本低

**劣势**:
- 版本受限于发行版
- 无法精确控制版本

### 3. 保持灵活性

**教训**: 为特殊平台准备备用方案  
**实践**: 
- 主方案：统一使用 aqt
- 备用方案：系统包 + 条件编译

### 4. 版本锁定的权衡

**问题**: 是否应该在所有平台锁定相同版本？

**答案**: 取决于场景
- **开发环境**: 可以宽松（>= 6.2）
- **生产环境**: 应该严格（= 6.6.3）
- **CI 环境**: 平衡（>= 6.5，尽量一致）

---

## 总结

### 本轮核心

**问题**: aqt 不支持 Linux ARM64  
**方案**: Ubuntu 24.04 系统包  
**效果**: 待验证（预期成功或需要 fallback）

### 优化历程进展

| 轮次 | 问题 | 修复 | 结果 |
|-----|------|------|------|
| 1-6 | 各种配置错误 | 逐步修复 | 其他平台成功 |
| 7 | Qt 版本 + 环境变量 | aqt 安装 | ❌ aqt 不支持 |
| 8 | aqt 不支持 ARM64 | Ubuntu 24.04 | ⏳ 进行中 |

### 下一步

- **如果成功**: 8 轮优化完成，所有平台通过 🎉
- **如果失败**: 应用 CMakeLists.txt fallback（第 9 轮）

---

*文档生成时间: 2025-11-18*  
*提交: 8a034cc*  
*运行: #19470431664*
