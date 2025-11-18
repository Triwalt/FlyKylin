---
description: 开发执行工作流 - 从User Story到可运行代码
auto_execution_mode: 3
---

# 开发执行工作流

## 使用场景
基于User Story和技术规格，实现具体功能代码。

## 前置条件
- [ ] User Story已编写并评审通过
- [ ] 技术规格已明确
- [ ] 开发环境已配置（运行过 `check-environment.ps1/sh`）
- [ ] 创建feature分支 `git checkout -b feature/US-XXX`

## 执行步骤

### 1. 理解需求和技术规格

```
@cascade 我准备实现以下User Story：

@docs/requirements/sprint_X/user_story_XXX.md
@docs/requirements/sprint_X/tech_spec_XXX.md

请帮我：
1. 总结关键需求和技术要点
2. 识别需要修改的模块和文件
3. 列出实现步骤（3-5步）
4. 指出潜在技术难点
```

### 2. 设计接口和类结构

// turbo
```
@cascade 基于技术规格，设计C++接口和类：

要求：
- 遵循项目架构原则（@.windsurf/rules/project-architecture.md）
- 接口类使用 I_ 前缀
- 成员变量使用 m_ 前缀
- 使用智能指针管理内存
- 考虑跨平台兼容（Windows/RK3566）

输出：
1. 类图或接口定义
2. 主要类的头文件声明
3. 依赖关系说明
```

### 3. 实现核心功能

// turbo
```
@cascade 实现以下类：

[类名和功能描述]

编码要求（@.windsurf/rules/cpp-coding-standards.md）：
- C++20标准，使用modern C++特性
- Qt 6.5+ API
- 遵循命名规范（PascalCase类名，camelCase函数名）
- 添加Doxygen注释
- 使用RAII管理资源
- 错误处理使用异常或std::optional
- 线程安全（如需要）

文件位置：
- 头文件：src/[module]/[class_name].h
- 源文件：src/[module]/[class_name].cpp
```

### 4. 实现平台特定代码（如需要）

```
@cascade 实现跨平台AI加速器：

Windows版本（DirectML）：
- 文件：src/platform/windows/directml_accelerator.h/cpp
- 使用ONNX Runtime DirectML EP
- 继承 I_Accelerator 接口

RK3566版本（NPU）：
- 文件：src/platform/rk3566/rknpu_accelerator.h/cpp
- 使用RKNN API
- 继承 I_Accelerator 接口

共同要求：
- 统一接口，隐藏平台细节
- 性能指标：Windows < 500ms, RK3566 < 1s
- 错误处理和日志记录
```

### 5. 编写单元测试

// turbo
```
@cascade 为以下类编写单元测试：

@src/[module]/[class_name].h

使用GoogleTest框架：
- 测试文件：tests/[module]/[class_name]_test.cpp
- 覆盖率目标：≥ 80%
- 测试用例包括：
  * 正常流程测试
  * 边界条件测试
  * 错误处理测试
  * 线程安全测试（如适用）
- Mock外部依赖（网络、文件系统、AI推理）

测试命名：TEST(ClassNameTest, MethodName_Scenario_ExpectedBehavior)
```

### 6. 本地验证

```bash
# 编译
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# 运行测试
ctest -V

# 代码格式化
cd ..
clang-format -i src/**/*.cpp src/**/*.h

# 静态分析
clang-tidy src/[module]/[file].cpp -- -std=c++20

# 内存检查（Linux）
valgrind --leak-check=full ./build/bin/test_executable
```

### 7. Git提交

```bash
# 查看修改
git status
git diff

# 添加文件
git add src/[module]/
git add tests/[module]/

# 提交（遵循规范）
git commit -m "feat(module): implement XXX feature

- Add [ClassName] for [功能]
- Implement [specific features]
- Add unit tests with 85% coverage

Closes #US-XXX"

# 推送到远程
git push origin feature/US-XXX
```

## Git Commit规范

格式：`<type>(<scope>): <subject>`

**Type类型**：
- `feat`: 新功能
- `fix`: Bug修复
- `docs`: 文档更新
- `style`: 代码格式（不影响功能）
- `refactor`: 重构
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建/工具配置

**Scope范围**：
- `core`: 核心模块
- `ai`: AI引擎
- `ui`: 界面层
- `platform`: 平台相关
- `storage`: 数据存储

**示例**：
```
feat(ai): implement NSFW detection with DirectML

- Add NsfwDetector class using MobileNetV3
- Support Windows DirectML acceleration
- Add unit tests with 90% coverage

Performance: < 500ms on average GPU

Closes #US-005
```

## 编码检查清单

提交PR前检查：

- [ ] 代码实现所有验收标准
- [ ] 遵循编码规范（命名、格式、注释）
- [ ] 使用智能指针，无内存泄漏
- [ ] 跨平台编译通过（Windows和Linux）
- [ ] 单元测试覆盖率 ≥ 80%
- [ ] 所有测试通过
- [ ] 通过clang-format格式化
- [ ] 通过clang-tidy静态分析
- [ ] 添加必要的Doxygen注释
- [ ] 更新相关文档

## 性能优化提示

### 内存优化
- 大对象传参使用 `const&` 或移动语义
- 容器使用 `reserve()` 预分配
- 避免不必要的拷贝
- RK3566注意内存限制（< 200MB）

### 性能优化
- 循环中避免重复计算
- 使用并发（QtConcurrent）处理耗时操作
- AI推理异步执行，不阻塞UI
- 数据库批量操作代替逐条操作

### 跨平台注意
- 文件路径使用 `QDir` 和 `QFileInfo`
- 网络操作使用Qt网络类
- 避免平台特定API（除platform目录）
- 使用条件编译隔离平台代码

## 常见问题

### Q: 如何选择智能指针？
- `std::unique_ptr`: 独占所有权
- `std::shared_ptr`: 共享所有权
- `QPointer`: Qt对象弱引用

### Q: 线程中使用Qt对象？
- 只在主线程操作UI对象
- 跨线程信号槽用 `Qt::QueuedConnection`
- 工作线程使用 `QtConcurrent::run()`

### Q: 如何Mock ONNX Runtime？
- 定义 `I_Accelerator` 接口
- 测试中用 `MockAccelerator` 实现
- 使用GoogleMock验证调用

## 相关文档

- `.windsurf/rules/cpp-coding-standards.md` - 编码规范
- `.windsurf/rules/project-architecture.md` - 架构原则
- `docs/编码规范.md` - 详细编码指南
- `.windsurf/workflows/code-review.md` - 代码审查流程