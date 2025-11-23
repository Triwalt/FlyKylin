---
description: 代码审查工作流 - 确保代码质量和架构一致性
auto_execution_mode: 3
---

# 代码审查工作流

## 使用场景
对Pull Request进行系统化代码审查，确保代码质量。

## 前置条件
- [ ] PR已创建并通过CI检查
- [ ] 开发者已自检（运行测试、格式化、静态分析）
- [ ] PR描述清晰（关联User Story、列出主要变更）

## 审查维度

### 1. 功能正确性（Critical）

```
@cascade 审查PR的功能实现：

@[PR文件列表]

对照User Story验证：
- [ ] 实现所有验收标准
- [ ] 边界条件处理正确
- [ ] 错误场景处理完善
- [ ] 功能逻辑符合需求

问题级别：
- Critical: 功能缺失或错误，必须修复
- Major: 功能不完整，强烈建议修复
```

### 2. 架构一致性（Critical）

```
@cascade 检查架构设计：

参考：@.windsurf/rules/project-architecture.md

审查要点：
- [ ] 遵循三层架构（UI/业务/数据）
- [ ] 模块划分合理（core/ai/platform/ui/storage）
- [ ] 依赖方向正确（单向依赖，避免循环）
- [ ] 使用正确的设计模式
- [ ] 平台相关代码正确隔离

问题级别：
- Critical: 违反架构原则
- Major: 模块职责不清
```

### 3. 编码规范（Major）

```
@cascade 检查编码规范：

参考：@.windsurf/rules/cpp-coding-standards.md

审查要点：
- [ ] 命名符合规范（类PascalCase，函数camelCase，成员m_前缀）
- [ ] 使用智能指针，无裸指针new/delete
- [ ] 遵循RAII原则
- [ ] C++20特性使用恰当
- [ ] Qt API使用正确
- [ ] 代码格式符合.clang-format
- [ ] 注释使用Doxygen风格

问题级别：
- Major: 严重不符合规范
- Minor: 小的规范问题
```

### 4. 性能优化（Major）

```
@cascade 分析性能影响：

审查要点：
- [ ] 无不必要的拷贝（使用const&或移动语义）
- [ ] 容器预分配（reserve）
- [ ] 避免循环中重复计算
- [ ] 异步处理耗时操作
- [ ] AI推理不阻塞UI
- [ ] RK3566平台性能达标

性能基准：
- Windows AI推理 < 500ms
- RK3566 AI推理 < 1s
- UI响应 < 100ms (Windows) / 200ms (RK3566)

问题级别：
- Major: 性能明显不达标
- Minor: 有优化空间
```

### 5. 线程安全（Critical）

```
@cascade 检查线程安全：

审查要点：
- [ ] 共享数据使用互斥锁保护
- [ ] 跨线程信号槽用QueuedConnection
- [ ] 避免死锁风险
- [ ] UI操作在主线程
- [ ] 工作线程使用QtConcurrent

问题级别：
- Critical: 存在数据竞争
- Major: 潜在线程安全问题
```

### 6. 错误处理（Major）

```
@cascade 检查错误处理：

审查要点：
- [ ] 检查所有可能失败的操作
- [ ] 异常使用合理（不可恢复的错误）
- [ ] 使用std::optional表示可选返回
- [ ] 日志记录完善（qCritical/qWarning）
- [ ] 用户友好的错误提示

问题级别：
- Critical: 未处理关键错误
- Major: 错误处理不完善
```

### 7. 测试充分性（Critical）

```
@cascade 审查测试代码：

@tests/[module]/

审查要点：
- [ ] 单元测试覆盖率 ≥ 80%
- [ ] 测试关键路径和边界条件
- [ ] 测试错误场景
- [ ] Mock外部依赖
- [ ] 测试命名清晰
- [ ] 所有测试通过

问题级别：
- Critical: 覆盖率不达标或关键功能未测试
- Major: 测试不够全面
```

## 审查流程

### Step 1: 快速浏览

```
@cascade 对这个PR进行快速概览：

@[PR链接或文件]

总结：
1. 主要变更内容（3-5点）
2. 修改的文件数量和代码行数
3. 总体复杂度评估
4. 初步印象（是否清晰、合理）
```

### Step 2: 详细审查

按照7个维度逐一审查，记录问题：

```
@cascade 详细审查以下文件：

@src/[module]/[file].cpp

审查维度：
1. 功能正确性
2. 架构一致性
3. 编码规范
4. 性能优化
5. 线程安全
6. 错误处理
7. 测试充分性

输出格式：
**文件**: src/[module]/[file].cpp

**Critical问题**:
- [行号] 问题描述 → 建议修复方案

**Major问题**:
- [行号] 问题描述 → 建议修复方案

**Minor问题**:
- [行号] 问题描述 → 建议修复方案

**优点**:
- 值得表扬的实现
```

### Step 3: 生成审查报告

```
@cascade 生成Code Review总结报告：

基于前面的审查，汇总：

# Code Review Report - PR #XXX

## 总体评价
□ Approve（批准合并）
□ Request Changes（要求修改）
□ Comment（仅评论）

## 问题汇总
**Critical**: X个（必须修复）
**Major**: X个（强烈建议修复）
**Minor**: X个（可选修复）

## Critical问题列表
1. [文件:行号] 问题描述
   - 影响：XXX
   - 建议：XXX

## Major问题列表
...

## 优点列表
1. XXX实现优雅
2. 测试覆盖全面

## 改进建议
1. 考虑使用XXX优化性能
2. 建议抽取XXX为独立函数

## 审查结论
[是否批准合并，需要哪些修改]
```

## 审查检查清单

使用此清单确保全面审查：

### 代码质量
- [ ] 功能实现正确
- [ ] 遵循编码规范
- [ ] 代码可读性好
- [ ] 适当的抽象层次
- [ ] 无重复代码

### 架构设计
- [ ] 符合项目架构
- [ ] 模块职责清晰
- [ ] 依赖关系合理
- [ ] 接口设计良好
- [ ] 扩展性考虑

### 安全性
- [ ] 输入验证完善
- [ ] 无缓冲区溢出
- [ ] 无SQL注入风险
- [ ] 敏感数据保护
- [ ] NSFW检测双端执行

### 性能
- [ ] 无明显性能问题
- [ ] 内存使用合理
- [ ] 算法复杂度适当
- [ ] 跨平台性能达标

### 可维护性
- [ ] 代码结构清晰
- [ ] 注释充分准确
- [ ] 命名有意义
- [ ] 易于理解
- [ ] 便于测试

### 测试
- [ ] 单元测试完整
- [ ] 覆盖率达标
- [ ] 测试质量高
- [ ] 集成测试（如需要）

## 常见问题模式

### 内存管理问题
```cpp
// ❌ 错误：裸指针容易泄漏
QFile* file = new QFile("data.txt");
if (!file->open(...)) return;  // 泄漏

// ✅ 正确：智能指针自动释放
auto file = std::make_unique<QFile>("data.txt");
if (!file->open(...)) return;  // 自动释放
```

### 线程安全问题
```cpp
// ❌ 错误：共享数据无保护
class Cache {
    std::map<int, Data> m_data;
    void insert(int key, Data value) {
        m_data[key] = value;  // 竞争条件
    }
};

// ✅ 正确：使用互斥锁
class Cache {
    std::mutex m_mutex;
    std::map<int, Data> m_data;
    void insert(int key, Data value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_data[key] = value;
    }
};
```

### 错误处理问题
```cpp
// ❌ 错误：未检查返回值
socket->bind(QHostAddress::Any, port);

// ✅ 正确：检查并处理错误
if (!socket->bind(QHostAddress::Any, port)) {
    qCritical() << "Failed to bind:" << socket->errorString();
    return false;
}
```

## 审查反馈模板

### 批准合并
```
✅ **Approve**

代码实现正确，质量良好，可以合并。

**优点**:
- 功能完整实现所有验收标准
- 架构设计合理，模块职责清晰
- 测试覆盖率达到85%
- 代码规范，可读性好

**Minor建议** (可在后续优化):
- 考虑将XXX抽取为独立方法提高复用性
- 可以添加更多边界测试用例

Great work! 🎉
```

### 要求修改
```
🔄 **Request Changes**

发现Critical问题，需要修复后才能合并。

**Critical问题** (必须修复):
1. [line 45] 共享数据m_peers未加锁保护
   → 建议：添加QMutex保护所有访问

2. [line 78] AI推理阻塞主线程
   → 建议：使用QtConcurrent::run()异步执行

**Major问题** (强烈建议):
1. [line 120] 未检查文件打开失败
   → 建议：添加错误处理和日志

请修复Critical问题后更新PR。
```

## 审查时间建议

- **小PR（< 200行）**: 15-30分钟
- **中PR（200-500行）**: 30-60分钟
- **大PR（> 500行）**: 建议拆分，或分多次审查

## 相关文档

- `.windsurf/rules/cpp-coding-standards.md` - 编码规范
- `.windsurf/rules/project-architecture.md` - 架构原则
- `.windsurf/workflows/development-execution.md` - 开发流程
- `docs/编码规范.md` - 详细编码指南