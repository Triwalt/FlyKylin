# FlyKylin 构建修复行动计划

**创建时间**: 2024-11-21  
**目标**: 在 2 小时内实现稳定的本地构建和测试运行  
**依赖**: [BUILD_ENVIRONMENT_DIAGNOSIS.md](./BUILD_ENVIRONMENT_DIAGNOSIS.md)

---

## ✅ 第一阶段：vcpkg 环境配置（30 min）

### 1.1 完成 D:/vcpkg 依赖安装

**当前状态**: 正在执行 `vcpkg install`（已启动，等待完成）

**执行命令**:
```powershell
cd D:\vcpkg

# 使用项目自定义 triplet
.\vcpkg.exe install qtbase protobuf abseil utf8-range gtest `
    --triplet x64-windows-release-only `
    --overlay-triplets E:\Project\FlyKylin\triplets
```

**预计耗时**: 20-30 分钟（首次安装）

**验证步骤**:
```powershell
# 检查安装结果
ls D:\vcpkg\installed\x64-windows-release-only\lib\

# 应该看到：
# - qtbase 相关 .lib
# - libprotobuf.lib
# - abseil_dll.lib
# - gtest.lib, gtest_main.lib
```

---

### 1.2 更新 CMakePresets.json 指向 D:/vcpkg

**文件**: `CMakePresets.json`

**修改内容**:
```json
{
  "name": "base-msvc",
  "hidden": true,
  "generator": "Visual Studio 17 2022",
  "architecture": "x64",
  "binaryDir": "${sourceDir}/build/${presetName}",
  "toolchainFile": "D:/vcpkg/scripts/buildsystems/vcpkg.cmake",  // ← 修改这里
  "cacheVariables": {
    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
    "VCPKG_TARGET_TRIPLET": "x64-windows-release-only",
    "VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/triplets"
  }
}
```

**验证**: 运行 `cmake --preset windows-release`，应该在 2 分钟内完成（不再下载依赖）

---

## ✅ 第二阶段：修复 CMake 配置（30 min）

### 2.1 修复 protocol/CMakeLists.txt

**文件**: `protocol/CMakeLists.txt`

**完整内容**:
```cmake
# Protocol library for Protobuf generated code

find_package(Protobuf CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(utf8_range CONFIG REQUIRED)

# Generate C++ code from .proto files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/messages.proto
)

# Use modern protobuf_generate
add_library(flykylin_protocol STATIC ${PROTO_FILES})

# Link Protobuf and its dependencies
# 注意：absl::log 包含 log_internal 实现
target_link_libraries(flykylin_protocol 
    PUBLIC 
        protobuf::libprotobuf
        absl::strings
        absl::base
        absl::log
        utf8_range::utf8_range
)

target_include_directories(flykylin_protocol
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate protobuf code
protobuf_generate(
    TARGET flykylin_protocol
    LANGUAGE cpp
    PROTOS ${PROTO_FILES}
)
```

---

### 2.2 修复根 CMakeLists.txt

**文件**: `CMakeLists.txt`

**删除冗余配置**:
```cmake
# 删除 line 58（硬编码 Qt 路径）
# set(CMAKE_PREFIX_PATH "D:/Qt/6.9.3/msvc2022_64" CACHE PATH "Qt installation path")

# 删除 line 72-86（Protobuf 查找，移到 protocol/CMakeLists.txt）
# if(VCPKG_INSTALLED_DIR)
#     set(Protobuf_DIR ...)
# endif()
# find_package(absl CONFIG REQUIRED)
# find_package(utf8_range CONFIG REQUIRED)
# find_package(Protobuf CONFIG REQUIRED)
# ...
```

**保留必要配置**:
```cmake
# 保留 Qt 查找
find_package(Qt6 6.4 REQUIRED COMPONENTS ...)

# 保留 ONNX Runtime（可选）
if(EXISTS "${ONNXRUNTIME_ROOT}/include")
    ...
endif()
```

---

### 2.3 确保 flykylin_core 包含 UI 源文件

**文件**: `src/core/CMakeLists.txt`

**确认以下行存在**:
```cmake
set(CORE_SOURCES
    # ... 其他源文件 ...
    
    # UI ViewModels（临时方案，后续应创建独立 flykylin_ui 库）
    ../ui/viewmodels/ChatViewModel.cpp
    ../ui/viewmodels/ChatViewModel.h
)

# 启用 AUTOMOC
set_target_properties(flykylin_core PROPERTIES
    AUTOMOC ON
)

# 链接 Qt Gui/Qml
target_link_libraries(flykylin_core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Qml
    flykylin_protocol
)
```

---

### 2.4 修复 tests/CMakeLists.txt

**文件**: `tests/CMakeLists.txt`

**启用测试文件**:
```cmake
set(TEST_SOURCES
    core/config/UserProfile_test.cpp
    core/ProtobufSerializer_test.cpp
    core/PeerDiscovery_test.cpp          # ← 取消注释
    # core/PeerNode_test.cpp              # TODO: 待实现
)

# 链接库顺序很重要
target_link_libraries(flykylin_tests PRIVATE
    GTest::gtest_main                    # ← main 必须在前
    GTest::gtest
    GTest::gmock
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Test
    Qt6::Qml
    flykylin_core
    flykylin_protocol
)
```

---

## ✅ 第三阶段：构建和测试（30 min）

### 3.1 清理并重新配置

```powershell
# 清理旧的构建文件（保留 vcpkg_installed）
Remove-Item -Recurse -Force build/windows-release/CMakeCache.txt -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force build/windows-release/CMakeFiles -ErrorAction SilentlyContinue

# 重新配置（应该很快，<2 min）
cmake --preset windows-release
```

**预期输出**:
```
-- Configuring done (XX.Xs)
-- Generating done (X.Xs)
-- Build files have been written to: E:/Project/FlyKylin/build/windows-release
```

---

### 3.2 编译 flykylin_protocol

```powershell
cmake --build --preset windows-release --target flykylin_protocol
```

**验证**:
- 应该生成 `build/windows-release/protocol/messages.pb.h` 和 `messages.pb.cc`
- 编译成功生成 `build/windows-release/lib/Release/flykylin_protocol.lib`
- 无链接错误

---

### 3.3 编译 flykylin_core

```powershell
cmake --build --preset windows-release --target flykylin_core
```

**验证**:
- 生成 MOC 文件（如 `moc_ChatViewModel.cpp`）
- 编译成功生成 `flykylin_core.lib`
- 无链接错误

---

### 3.4 编译和运行测试

```powershell
# 编译测试
cmake --build --preset windows-release --target flykylin_tests

# 运行测试
ctest --preset windows-release --output-on-failure
```

**预期结果**:
```
Test project E:/Project/FlyKylin/build/windows-release
    Start 1: flykylin_tests
1/1 Test #1: flykylin_tests ...................   Passed    0.XX sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.XX sec
```

---

### 3.5 问题排查清单

如果测试失败，按以下顺序检查：

**链接错误（absl 符号未解析）**:
- ✅ 检查 `protocol/CMakeLists.txt` 是否链接了 `absl::log`
- ✅ 检查 `D:/vcpkg/installed/x64-windows-release-only/lib/` 是否有 `abseil_dll.lib`
- ✅ 尝试添加更多 absl 组件：`absl::log_internal_message`, `absl::log_internal_check_impl`

**Qt MOC 错误（staticMetaObject 未定义）**:
- ✅ 检查 `src/core/CMakeLists.txt` 是否启用了 `AUTOMOC`
- ✅ 检查 `ChatViewModel.cpp` 是否在 `CORE_SOURCES` 中
- ✅ 检查 `flykylin_core` 是否链接了 `Qt6::Gui` 和 `Qt6::Qml`

**GTest 链接错误（MakeAndRegisterTestInfo 未定义）**:
- ✅ 检查 `tests/CMakeLists.txt` 的链接顺序：`GTest::gtest_main` 必须在 `GTest::gtest` 之前

**DLL 缺失**:
```powershell
# 手动复制 DLL 到测试目录
Copy-Item D:\vcpkg\installed\x64-windows-release-only\bin\*.dll `
    -Destination build\windows-release\bin\Release\
```

---

## ✅ 第四阶段：验证和提交（30 min）

### 4.1 完整构建测试

```powershell
# 清理并完整构建
Remove-Item -Recurse -Force build/windows-release -ErrorAction SilentlyContinue
cmake --preset windows-release
cmake --build --preset windows-release
ctest --preset windows-release --output-on-failure
```

**成功标准**:
- [x] CMake 配置 < 5 分钟
- [x] 编译无错误
- [x] 测试全部通过
- [x] 无链接警告

---

### 4.2 更新文档

**文件**: `README.md`

**添加快速开始章节**:
```markdown
## 快速开始

### 前置要求
- Visual Studio 2022 (MSVC)
- CMake ≥ 3.20
- vcpkg（推荐使用 D:/vcpkg）

### 构建步骤

1. 安装依赖（首次）
```powershell
cd D:\vcpkg
.\vcpkg.exe install qtbase protobuf abseil utf8-range gtest `
    --triplet x64-windows-release-only `
    --overlay-triplets E:\Project\FlyKylin\triplets
```

2. 配置和构建
```powershell
cd E:\Project\FlyKylin
cmake --preset windows-release
cmake --build --preset windows-release
```

3. 运行测试
```powershell
ctest --preset windows-release
```
```

---

### 4.3 提交修改

```powershell
git add .
git commit -m "fix(build): 修复构建环境和依赖链接问题

- 迁移到独立 vcpkg 目录（D:/vcpkg）加速构建
- 修复 protocol/CMakeLists.txt 的 absl 依赖链接
- 移除 CMakeLists.txt 硬编码路径
- 启用 PeerDiscovery 测试
- 更新构建文档

Refs: C-001, C-002, C-004 in BUILD_ENVIRONMENT_DIAGNOSIS.md"
```

---

## 📊 进度追踪

| 阶段 | 任务 | 预计时间 | 状态 | 实际时间 |
|------|------|----------|------|----------|
| 1 | vcpkg 安装 | 30 min | 🔄 进行中 | - |
| 1 | 更新 preset | 5 min | ⏸️ 待开始 | - |
| 2 | 修复 protocol CMake | 10 min | ⏸️ 待开始 | - |
| 2 | 修复根 CMake | 10 min | ⏸️ 待开始 | - |
| 2 | 修复测试 CMake | 10 min | ⏸️ 待开始 | - |
| 3 | 重新配置构建 | 5 min | ⏸️ 待开始 | - |
| 3 | 编译测试 | 10 min | ⏸️ 待开始 | - |
| 3 | 运行测试 | 5 min | ⏸️ 待开始 | - |
| 4 | 验证 | 10 min | ⏸️ 待开始 | - |
| 4 | 文档更新 | 10 min | ⏸️ 待开始 | - |
| 4 | 提交代码 | 10 min | ⏸️ 待开始 | - |
| **总计** | - | **2 小时** | - | - |

---

## 🎯 成功指标

构建修复完成的标准：

1. ✅ **构建速度**: 清理后重新配置 < 5 分钟
2. ✅ **编译成功**: 无错误，警告 < 10 个
3. ✅ **测试通过**: 至少 3 个测试（UserProfile, ProtobufSerializer, PeerDiscovery）全部通过
4. ✅ **CI 兼容**: 本地配置可移植到 CI（下一步工作）

---

## 🚀 后续优化计划

完成本次修复后的改进方向：

1. **CI 配置同步**（Sprint 3）
   - 更新 `.github/workflows/build-and-release.yml` 使用相同 vcpkg 策略
   - 统一 Qt 版本到 6.6.3 或 6.9.3

2. **测试扩展**（Sprint 3）
   - 添加 `MessageService_test.cpp`
   - 添加 `ChatViewModel_test.cpp`
   - 提高代码覆盖率到 60%+

3. **架构清理**（Sprint 4）
   - 创建独立 `flykylin_ui` 库
   - 移除 `flykylin_core` 中的 UI 代码
   - 实现真正的分层架构

---

**维护人**: AI Cascade  
**审查周期**: 每次构建问题修复后  
**文档版本**: 1.0  
**最后更新**: 2024-11-21
