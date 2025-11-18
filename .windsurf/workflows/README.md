# Windsurf 工作流目录

本目录包含FlyKylin项目的标准化工作流程定义。

## 📚 可用工作流

### 需求与规划
- `/requirements-analysis` - 需求分析工作流：从原始需求到User Story
- `/sprint-planning` - Sprint规划工作流：制定可执行的迭代计划

### 开发与测试
- `/development-execution` - 开发执行工作流：从User Story到可运行代码
- `/code-review` - 代码审查工作流：确保代码质量和架构一致性

### 交付与发布
- `/stage-delivery` - **阶段性交付工作流**：Sprint结束时的工作整理、构建与发布 ⭐

## 🎯 工作流使用方式

在Windsurf中通过 `@[/workflow-name]` 调用工作流，例如：

```
@[/stage-delivery] 执行Sprint 1的阶段性发布
```

或使用自动化脚本：

```powershell
# 执行阶段性发布
.\tools\agent\stage-release.ps1 -Version "0.1.0" -SprintNum 1

# 参数说明
-Version        版本号 (必需)
-SprintNum      Sprint编号 (必需)
-SkipTests      跳过测试
-SkipQualityCheck  跳过质量检查
-DryRun         模拟运行（不执行实际操作）
```

## 📋 工作流结构

每个工作流文件（.md）包含：

```markdown
---
description: 工作流简短描述
---

# 工作流标题

## 使用场景
说明何时使用此工作流

## 前置条件
- [ ] 条件1
- [ ] 条件2

## 执行步骤

### 第1步：...
// turbo  # 表示可以自动执行的步骤
```
...
```

## ✅ 检查清单
- [ ] 检查项1
- [ ] 检查项2
```

## 🔗 相关资源

- Agent配置：`agent_prompts.yaml`
- 项目架构规则：`.windsurf/rules/project-architecture.md`
- 编码规范：`.windsurf/rules/cpp-coding-standards.md`
- 文档管理：`.windsurf/rules/document-management.md`

## 📝 工作流维护

- **所有者**：Orchestrator Agent
- **更新频率**：根据项目需要持续优化
- **反馈渠道**：通过项目Issue或团队讨论

## 🎓 最佳实践

1. **按顺序执行**：工作流步骤有依赖关系，建议按顺序执行
2. **检查前置条件**：执行前确保满足所有前置条件
3. **记录执行日志**：保留执行过程的输出用于追溯
4. **持续改进**：根据实际使用反馈优化工作流

---

**最后更新**：2024-11-18  
**版本**：1.0
