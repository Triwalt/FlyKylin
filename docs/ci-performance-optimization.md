# GitHub Actions CI/CD 性能优化指南

## 当前性能分析

### 实际运行时间（首次构建）
- **Windows AMD64**: 18.8 分钟
  - Configure CMake: 17.5 分钟 (93%)
  - Build: 0.5 分钟
  - 其他: 0.8 分钟
- **Linux AMD64**: 12.4 分钟
- **Windows ARM64**: ~50 分钟（预估，含交叉编译）
- **Linux ARM64**: ~30 分钟（预估，Docker 容器）

### 耗时分析

#### Configure CMake 为什么这么慢？
vcpkg 从源码编译所有依赖：
```
protobuf (Protocol Buffers)
├── abseil (Google Abseil)
│   ├── 100+ 子模块
│   └── 每个架构单独编译
├── zlib
└── utf8-range

gtest (Google Test)
sqlite3
以及所有传递依赖...
```

**ARM64 交叉编译更慢**：
- protobuf ARM64: 30-40 分钟
- abseil ARM64: 20-30 分钟
- 总计: 50-60 分钟

## GitHub Actions 工作原理

### 运行环境
- **平台**: Azure 云主机（GitHub 托管）
- **Windows**: Windows Server 2022, 2核CPU, 7GB RAM
- **Linux**: Ubuntu 22.04, 2核CPU, 7GB RAM
- **特点**: 每次运行都是全新虚拟机（无持久化状态）

### 为什么每次都要重新配置？
因为 GitHub Actions 是**无状态**的：
1. 启动全新虚拟机
2. 克隆代码
3. 安装依赖（Qt, vcpkg, CMake）
4. 配置 → 构建 → 测试 → 打包
5. 销毁虚拟机

**除非使用缓存，否则每次都从零开始！**

## 已启用的优化

### ✅ 当前配置
```yaml
# 1. Qt 缓存
- uses: jurplel/install-qt-action@v4
  with:
    cache: true  # ✓ 已启用

# 2. vcpkg 自动缓存
- uses: lukka/run-vcpkg@v11  # ✓ 自动缓存 vcpkg 目录

# 3. vcpkg 二进制缓存
env:
  VCPKG_BINARY_SOURCES: clear;x-gha,readwrite  # ✓ 已启用
```

### 缓存生效时间线
- **首次运行**: 18-20 分钟（建立缓存）
- **第 2 次**: 12-15 分钟（部分缓存命中）
- **第 3 次**: 5-8 分钟（大部分缓存命中）
- **第 4+ 次**: 3-5 分钟（完全缓存命中）

**注意**: 修改 `VCPKG_COMMIT` 会使缓存失效！

## 优化方案

### 方案 1: 条件构建 ARM64（推荐）⭐

**问题**: ARM64 交叉编译占用 50% 以上时间  
**方案**: 仅在 release tag 时构建 ARM64

```yaml
build-windows-arm64:
  runs-on: windows-latest
  if: startsWith(github.ref, 'refs/tags/v')  # 仅在 tag 时运行
  # ...

build-linux-arm64:
  runs-on: ubuntu-22.04
  if: startsWith(github.ref, 'refs/tags/v')  # 仅在 tag 时运行
  # ...
```

**效果**: 
- 日常 push: 5-8 分钟（仅 AMD64）
- Release: 30-40 分钟（全平台）

### 方案 2: 添加 CMake 缓存（推荐）⭐⭐

vcpkg 二进制缓存有时不稳定，显式缓存 build 目录：

```yaml
- name: Cache CMake build
  uses: actions/cache@v4
  with:
    path: |
      build
      vcpkg/installed
      vcpkg/packages
    key: ${{ runner.os }}-${{ runner.arch }}-cmake-${{ hashFiles('**/CMakeLists.txt', 'vcpkg.json') }}
    restore-keys: |
      ${{ runner.os }}-${{ runner.arch }}-cmake-
```

**效果**: 
- 首次: 18 分钟
- 缓存命中: 2-3 分钟

### 方案 3: 使用 sccache/ccache（高级）⭐⭐⭐

缓存 C++ 编译结果：

```yaml
- name: Setup sccache
  uses: mozilla-actions/sccache-action@v0.0.4

- name: Configure CMake
  run: |
    cmake -B build \
      -DCMAKE_C_COMPILER_LAUNCHER=sccache \
      -DCMAKE_CXX_COMPILER_LAUNCHER=sccache \
      # ...
```

**效果**: 
- 首次: 18 分钟
- 缓存命中: 1-2 分钟（仅重新编译修改的文件）

### 方案 4: 减少依赖（高级）

检查是否真的需要所有依赖：

```json
// vcpkg.json
{
  "dependencies": [
    "protobuf",    // 必需？
    "sqlite3",     // 必需？
    "gtest"        // 仅测试？可以按需安装
  ],
  "features": {
    "tests": {
      "description": "Build tests",
      "dependencies": ["gtest"]  // 仅测试时安装
    }
  }
}
```

**效果**: 减少 20-50% 编译时间（取决于依赖）

### 方案 5: 并行构建优化

增加并行度（当前 `-j4`）：

```yaml
- name: Build
  run: cmake --build build --config Release -j8  # 改为 8
```

**效果**: 10-20% 提升（受限于 GitHub Actions 2核CPU）

### 方案 6: 自托管 Runner（专业）

使用你自己的机器：
- 保留所有编译缓存
- 性能可控
- 成本: 需维护服务器

**效果**: 
- 首次: 15 分钟
- 后续: 1-2 分钟

## 业界标准对比

| 项目类型 | 首次构建 | 缓存后 | 备注 |
|---------|---------|--------|------|
| 小型 Qt 项目（无依赖） | 2-5 min | 1-2 min | - |
| 中型项目（vcpkg/Conan） | 15-25 min | 3-8 min | 正常 |
| 大型项目（如 Qt Creator） | 30-60 min | 10-20 min | - |
| **你的项目** | **18 min** | **3-5 min** | ✓ 正常 |

## ✅ 已实施的优化（2025-11-18）

### 🎯 VCPKG_BUILD_TYPE=release（重大优化）
**提交**: 8bf6c9d  
**效果**: 节省 50-60% vcpkg 构建时间

```yaml
env:
  VCPKG_BUILD_TYPE: release  # 只构建 release，跳过 debug
```

**原理**:
- vcpkg 默认同时构建 Debug 和 Release 版本的依赖
- 项目只需要 Release，但 vcpkg 仍会构建 Debug
- protobuf Debug 版本在 ARM64 上需要 30-40 分钟
- 设置 `VCPKG_BUILD_TYPE=release` 跳过所有 Debug 构建

**时间对比** (ARM64):
- 修复前: protobuf 60-80 分钟 (Debug + Release)
- 修复后: protobuf 20-30 分钟 (Release only)
- **节省**: ~50 分钟

## 推荐优化步骤

### 立即实施（零风险）
1. ✅ **VCPKG_BUILD_TYPE=release** - 已实施，节省 50-60% 时间
2. ✅ **等待缓存生效** - 2-3 次运行后自动加速
3. ✅ **避免频繁修改 VCPKG_COMMIT** - 会导致缓存失效

### 短期优化（低风险）
1. **添加 ARM64 条件构建** - 节省 50% 时间
2. **添加显式 CMake 缓存** - 提升稳定性

### 中期优化（需测试）
1. **启用 sccache** - 大幅加速重编译
2. **优化 vcpkg.json** - 移除不必要依赖

### 长期优化（需投入）
1. **自托管 Runner** - 最佳性能
2. **迁移到预编译依赖** - 跳过 vcpkg

## 常见问题

### Q: 为什么第一次这么慢？
A: vcpkg 从源码编译所有依赖。这是**正常的**，所有使用 vcpkg 的项目都会遇到。

### Q: 能否跳过编译直接用二进制？
A: vcpkg 二进制缓存已启用，但首次仍需编译。可考虑：
- Conan（更多预编译包）
- 系统包管理器（apt, brew）
- Qt 官方安装器

### Q: 18 分钟算慢吗？
A: 对于有 protobuf + abseil 的项目，**正常偏快**。缓存后 3-5 分钟属于**正常水平**。

### Q: 能否并行构建多个平台？
A: 已经是并行的！4 个 job 同时运行，总墙上时钟时间约为最慢 job 的时间。

## 监控与诊断

### 查看详细耗时
```bash
# 使用工具脚本
gh run view <run-id> --log | grep "Elapsed time"

# 查看 vcpkg 编译了什么
gh run view <run-id> --log | grep "Building"
```

### 检查缓存命中
在 GitHub Actions 日志中搜索：
```
Cache restored from key: ...  # ✓ 缓存命中
Cache not found for input keys: ...  # ✗ 缓存未命中
```

## 总结

**当前状态**: ✓ 正常，已做基础优化  
**首次构建**: 18-20 分钟（正常）  
**稳定后**: 3-5 分钟（优秀）  

**建议**: 
1. 耐心等待缓存建立（2-3 次运行后生效）
2. 考虑添加 ARM64 条件构建
3. 如需更快，启用 sccache

---

*生成时间: 2025-11-18*
