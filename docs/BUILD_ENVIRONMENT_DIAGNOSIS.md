# FlyKylin 项目诊断报告

**生成时间**: 2024-11-21  
**诊断范围**: 构建环境、项目结构、代码质量、文档一致性  
**状态**: 🔴 Critical - 需要立即处理多个阻塞性问题

---

## 🔴 Critical 级别问题（阻塞构建）

### C-001: CMake 配置中的硬编码绝对路径

**位置**: `CMakeLists.txt:58, 105-106`

```cmake
# 硬编码的Qt路径
set(CMAKE_PREFIX_PATH "D:/Qt/6.9.3/msvc2022_64" CACHE PATH "Qt installation path")

# 硬编码的模型路径
set(NSFW_MODEL_PATH "E:/Project/tensorflow-open_nsfw/open_nsfw.onnx" CACHE PATH "NSFW detection model")
set(BGE_MODEL_PATH "E:/Project/tensorflow-open_nsfw/onnx_models/bge-small-zh-v1.5/bge-small-zh-v1.5.onnx" CACHE PATH "BGE semantic search model")
```

**问题**:
- 路径绑定到特定机器，CI 和其他开发者无法使用
- Qt 路径同时在 `CMakeLists.txt` 和 `CMakePresets.json` 中定义，造成冲突
- 模型路径指向项目外部，依赖不可追溯

**影响**: CI 构建失败、多人协作困难、路径不存在时配置失败

**建议修复**:
```cmake
# 移除硬编码，依赖 CMakePresets.json 或环境变量
# 删除 line 58 的 CMAKE_PREFIX_PATH
# 模型路径改为相对路径或可选
if(NOT NSFW_MODEL_PATH)
    set(NSFW_MODEL_PATH "${CMAKE_SOURCE_DIR}/models/open_nsfw.onnx" CACHE PATH "NSFW detection model")
endif()
```

---

### C-002: Protobuf/Abseil 链接依赖缺失

**位置**: `protocol/CMakeLists.txt:14-17`

**问题**:
- `flykylin_protocol` 只链接 `protobuf::libprotobuf`，没有显式链接 `absl` 和 `utf8_range`
- Protobuf 生成的代码使用了 `absl::log_internal::LogMessage` 等符号，但传递依赖不完整
- 导致 `flykylin_tests` 链接时出现 3+ 个未解析符号错误

**当前症状**:
```
error LNK2019: 无法解析的外部符号 "public: void __cdecl absl::lts_20250814::log_internal::LogMessage::Flush(void)"
```

**建议修复**:
```cmake
# protocol/CMakeLists.txt
find_package(absl CONFIG REQUIRED)
find_package(utf8_range CONFIG REQUIRED)

target_link_libraries(flykylin_protocol 
    PUBLIC 
        protobuf::libprotobuf
        absl::strings
        absl::base
        absl::log
        absl::log_internal_message
        utf8_range::utf8_range
)
```

---

### C-003: CI 配置与本地环境严重不一致

**位置**: `.github/workflows/build-and-release.yml`

**问题对比表**:

| 配置项 | CI | 本地 | 影响 |
|--------|-----|------|------|
| Qt 版本 | 6.6.3 | 6.9.3 | 不同版本可能导致 API 差异 |
| 测试路径 | `cd build` | `build/windows-release` | CI 测试永远失败 |
| Preset 使用 | 部分使用 | 完全依赖 | 配置不一致 |
| VCPKG_BUILD_TYPE | 环境变量 | CMake 变量 | vcpkg 无法识别 |

**CI 错误示例**:
```yaml
# Line 80-81: 错误的测试路径
- name: Run tests
  run: |
    cd build  # ❌ 应该是 build/windows-release
    ctest -C ${{ env.BUILD_TYPE }} --output-on-failure
```

**建议修复**:
1. 统一 Qt 版本到 6.9.3 或降级本地到 6.6.3
2. 修正测试路径：`ctest --preset windows-release`
3. 传递 VCPKG_BUILD_TYPE 为 CMake 变量：`-DVCPKG_BUILD_TYPE=release`

---

### C-004: vcpkg 重复安装导致构建极慢

**现状**: 每次清空 `build/windows-release` 后，`cmake --preset` 需要 60+ 分钟重新下载/编译所有依赖

**根因**:
1. vcpkg 依赖安装在 `build/windows-release/vcpkg_installed/`，清理 build 会删除缓存
2. 项目使用自带 `vcpkg/` 子模块，没有启用全局缓存或二进制源
3. 没有配置 `VCPKG_BINARY_SOURCES` 或使用独立 vcpkg 目录

**建议修复方案（3选1）**:

**方案A: 使用独立 vcpkg 目录（推荐）**
```cmake
# CMakePresets.json 改为指向 D:/vcpkg
"toolchainFile": "D:/vcpkg/scripts/buildsystems/vcpkg.cmake"
```
优点：依赖安装在 `D:/vcpkg/installed/`，清理 build 不影响
缺点：需要提前手动安装依赖

**方案B: 启用 vcpkg 二进制缓存**
```cmake
# CMakePresets.json 添加
"cacheVariables": {
    "VCPKG_BINARY_SOURCES": "default,clear;files,${sourceDir}/.vcpkg-cache,readwrite"
}
```
优点：首次安装后缓存复用
缺点：`.vcpkg-cache` 目录会很大（需加入 .gitignore）

**方案C: 不清理 build 目录**
只删除 `CMakeCache.txt` 和 `CMakeFiles/`，保留 `vcpkg_installed/`

---

## 🟠 High 级别问题（严重影响）

### H-001: 测试覆盖率极低且大量注释

**位置**: `tests/CMakeLists.txt:14-19`

```cmake
set(TEST_SOURCES
    core/config/UserProfile_test.cpp
    core/ProtobufSerializer_test.cpp
    # core/PeerNode_test.cpp  # TODO: 待实现
    # core/PeerDiscovery_test.cpp  # TODO: 待实现
)
```

**问题**:
- 只有 2 个测试文件启用，其他都被注释
- `ChatViewModel_test.cpp`、`MessageService_test.cpp` 在之前会话中创建但未加入
- `PeerDiscovery_test.cpp` 已存在但被注释掉（TODO: 待实现）
- 测试无法验证核心功能（P2P 发现、消息收发）

**建议**:
1. 启用 `PeerDiscovery_test.cpp`（代码已存在）
2. 添加 `MessageService_test.cpp` 和 `ChatViewModel_test.cpp`
3. 修复链接问题后逐步启用所有测试

---

### H-002: 根 CMakeLists.txt 依赖查找顺序混乱

**位置**: `CMakeLists.txt:72-86`

**问题**:
- 先 `find_package(Protobuf)`，再在 `protocol/CMakeLists.txt` 里又查找一次（重复）
- `absl` 和 `utf8_range` 在根 CMakeLists 查找，但只在 protocol 里使用
- `include_directories(${Protobuf_INCLUDE_DIRS})` 全局污染（现代 CMake 应该用 target-based）

**建议**:
```cmake
# 根 CMakeLists.txt 删除 Protobuf 相关查找
# 删除 line 72-86 的 Protobuf/absl/utf8_range 查找

# protocol/CMakeLists.txt 里统一处理
find_package(Protobuf CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(utf8_range CONFIG REQUIRED)
# 使用 target_link_libraries 和 target_include_directories
```

---

### H-003: Qt MOC 文件生成但未正确链接

**问题**: `CMAKE_AUTOMOC ON` 已启用，但某些 QObject 派生类的 MOC 文件未被链接到测试

**症状**:
```
error LNK2019: 无法解析的外部符号 "public: static struct QMetaObject const flykylin::ui::ChatViewModel::staticMetaObject"
```

**根因**: `ChatViewModel.cpp` 在 `src/ui/viewmodels/` 但未加入任何 CMake target

**临时修复**（之前会话）: 将 `ChatViewModel.cpp` 加入 `flykylin_core`
**正确做法**: 创建 `flykylin_ui` 库或明确组织 UI 相关源文件

---

### H-004: 架构文档与实际代码严重脱节

**位置**: `ARCHITECTURE_PROPOSAL.md`, `docs/architecture/adr/001-architecture-modernization.md`

**问题**:
- 文档声称"已实现六角架构"，但代码仍是传统 Qt 信号槽
- 文档提到 `QCoro` 协程，但实际只有 `FetchContent` 声明，未真正使用
- 空目录 `src/core/interfaces/`, `src/core/ports/` 表明接口层未实现
- ADR-001 描述的架构目标与当前 Sprint 2 交付物不符

**建议**:
1. 重命名 `ARCHITECTURE_PROPOSAL.md` 为 `ARCHITECTURE_FUTURE.md` 并加上 **"未实现"** 警告
2. 创建 `ARCHITECTURE_CURRENT.md` 描述实际架构（Qt Widgets + 信号槽 + Protobuf）
3. 清理空目录或明确标注"预留目录"

---

## 🟡 Medium 级别问题（需改进）

### M-001: 代码中大量 TODO 和占位符

**统计**: 在 `src/` 目录发现 4+ 个 TODO 注释

**示例**:
```cpp
// src/ui/windows/MainWindow.cpp:153
// TODO: 更新UI显示选中用户信息

// src/ui/windows/ChatWindow.cpp:204
// TODO: Show error indicator on message bubble

// src/ui/windows/ChatWindow.cpp:242
// TODO: Get local user ID properly
```

**建议**: 将 TODO 转换为 GitHub Issues 或技术债务条目，避免代码中残留

---

### M-002: 日志使用不规范

**问题**: 全部使用 `qDebug()` 无法分级控制，Release 模式仍会输出

**建议**: 使用 `Q_LOGGING_CATEGORY` 按模块分类，支持运行时开关

---

### M-003: 文档冗余和过时

**问题**:
- `docs/ci-*` 系列有 5+ 个文档记录 CI 修复历史，缺少汇总
- Sprint 文档散落在多处（`docs/requirements/`, `docs/sprints/`）
- 技术债务文档 `technical-debt.md` 与实际状态不同步（如 TD-010/TD-011 已部分实现）

**建议**:
1. 创建 `docs/ci/CHANGELOG.md` 汇总所有 CI 优化
2. Sprint 文档统一放在 `docs/sprints/sprint_X/`
3. 每个 Sprint 结束时同步更新 `technical-debt.md`

---

## 🟢 Low 级别问题（优化建议）

### L-001: .gitignore 可能未覆盖所有生成文件

建议检查是否忽略：
- `build/`
- `.vcpkg-cache/` （如启用二进制缓存）
- `.vs/`, `.vscode/`, `.idea/`
- `*.user`, `CMakeLists.txt.user`

---

### L-002: 缺少 CONTRIBUTING.md 和 DEVELOPMENT.md

新开发者不清楚：
- 如何配置环境
- 如何运行测试
- 代码风格要求
- PR 流程

建议创建这些文档

---

### L-003: CMake 编译选项可以优化

```cmake
# 当前
if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
endif()

# 建议增加
if(MSVC)
    add_compile_options(/W4 /WX /permissive- /utf-8)  # /WX: 警告视为错误
    add_compile_options($<$<CONFIG:Release>:/O2 /GL>)  # 全局优化
endif()
```

---

## 📋 推荐修复优先级

### 阶段 1: 紧急修复（本次会话完成）
1. ✅ **C-004**: 配置独立 vcpkg 目录（D:/vcpkg）- 正在进行
2. 🔲 **C-002**: 修复 Protobuf/absl 链接
3. 🔲 **H-003**: 确保 Qt MOC 正确生成和链接
4. 🔲 构建并运行 `flykylin_tests`

### 阶段 2: 环境一致性（后续 Sprint）
1. 🔲 **C-001**: 移除硬编码路径
2. 🔲 **C-003**: 修复 CI 配置使其与本地一致
3. 🔲 **H-001**: 启用所有测试文件

### 阶段 3: 架构和文档（技术债务清理）
1. 🔲 **H-004**: 更新架构文档
2. 🔲 **M-002**: 日志系统规范化
3. 🔲 **M-003**: 文档整理和汇总

---

## 🎯 关键决策建议

### 决策1: vcpkg 管理策略

**选项A（推荐）**: 使用 `D:/vcpkg` 独立目录
- ✅ 本地构建快（5-10 min）
- ✅ 清理 build 不影响
- ❌ CI 需要预装或缓存

**选项B**: 保持项目内 vcpkg 但启用二进制缓存
- ✅ CI 和本地统一
- ❌ 首次仍慢，缓存管理复杂

**建议**: 先用选项A 稳定本地开发，CI 后续优化

---

### 决策2: Qt 版本

**选项A**: 统一到 6.6.3（CI 版本）
- ✅ CI 和本地完全一致
- ❌ 无法使用 Qt 6.9 新特性

**选项B**: 统一到 6.9.3（本地版本）
- ✅ 使用最新特性
- ❌ 需要更新 CI 的 Qt 安装步骤

**建议**: 选 A，稳定性优先

---

### 决策3: 架构重构时机

**当前状态**: Sprint 2 完成基本功能，但架构仍是传统模式

**建议**: **不要急于重构**
1. 先完成 Sprint 3-4 的核心功能（文件传输、AI 集成）
2. 在功能稳定后（Sprint 5+）再考虑架构现代化
3. 重构应增量进行，不要"推倒重来"

---

## 📊 诊断总结

| 类别 | Critical | High | Medium | Low | 总计 |
|------|----------|------|--------|-----|------|
| 数量 | 4 | 4 | 3 | 3 | 14 |
| 预计工作量 | 12h | 16h | 8h | 4h | 40h |

**关键瓶颈**: Protobuf/absl 链接问题和 vcpkg 安装速度

**下一步行动**:
1. ✅ 完成 `D:/vcpkg` 安装和配置
2. 修复 `protocol/CMakeLists.txt` 的依赖链接
3. 重新构建并运行测试
4. 验证 CI 配置

---

**维护人**: AI Cascade  
**审查周期**: 每 Sprint 开始前  
**文档版本**: 1.0  
**最后更新**: 2024-11-21
