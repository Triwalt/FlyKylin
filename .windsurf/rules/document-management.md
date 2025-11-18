---
trigger: always_on
description: Agent文档管理规则 - 精简、结构化、易维护
globs: docs/**
---

# Agent文档管理规则

<document_organization>

## 目录结构约束

**核心原则**：精简文档，只保存关键产出，避免冗余。

### 必需目录结构
```
docs/
├── requirements/          # 需求文档
│   ├── backlog/          # Product Backlog (US-XXX_*.md)
│   └── archive/          # 已完成需求归档
├── sprints/              # Sprint管理
│   ├── active/           # 当前和最近2个Sprint
│   └── archive/          # 历史Sprint归档
├── architecture/         # 架构文档
│   ├── adr/             # Architecture Decision Records
│   └── interfaces/      # 接口设计文档
├── code-review/         # Code Review报告 (按月归档)
│   └── YYYY-MM/
└── templates/           # 文档模板
```

</document_organization>

<minimalist_principles>

## 精简文档原则

### 保留的文档类型
✅ **必须保留**:
- User Story (关键需求)
- Tech Spec (技术方案)
- Sprint Plan (迭代计划)
- Code Review (质量记录)
- ADR (架构决策)

### 简化的文档类型
📝 **合并简化**:
- Daily Standup → 合并到 Sprint `notes.md`
- 任务分解 → 集成到 Sprint `plan.md`
- 测试报告 → 集成到 Code Review
- Bug追踪 → 使用Git Issues，不单独建文档

### 不创建的文档
❌ **避免冗余**:
- 会议记录（非正式讨论）
- 临时笔记
- 重复的架构图
- 过细的任务清单
- 进度跟踪表格

</minimalist_principles>

<agent_guidance>

## Agent输出文档指南

### 需求分析Agent
**输出位置**: `docs/requirements/backlog/`

### Sprint规划Agent
**输出位置**: `docs/sprints/active/sprint_X/`
- 进度笔记: `notes.md` (简化日志)

### 开发执行Agent
**输出位置**: 代码注释、PR描述
- 不生成独立文档
- 技术细节写在代码Doxygen注释
- 实现说明写在Git commit和PR描述

### 代码审查Agent
**输出位置**: `docs/code-review/YYYY-MM/`
- 关键问题摘要，避免逐行记录

### 架构设计Agent
**输出位置**: `docs/architecture/`
- 接口文档: `interfaces/{模块名}.md`

</agent_guidance>

<quality_checks>

## 文档质量检查

### 命名规范
```bash
# User Story
docs/requirements/backlog/US-[0-9]{3}_*.md

# Tech Spec
docs/requirements/backlog/TechSpec-[0-9]{3}.md

# Sprint Plan
docs/sprints/active/sprint_[0-9]+/plan.md

# Code Review
docs/code-review/[0-9]{4}-[0-9]{2}/PR-[0-9]+-[0-9]{4}-[0-9]{2}-[0-9]{2}.md

# ADR
docs/architecture/adr/[0-9]{3}-*.md
```
### 内容
- [ ] User Story包含验收标准
- [ ] Tech Spec关联User Story
- [ ] Sprint Plan包含目标和DoD
- [ ] Code Review包含审查结论
- [ ] ADR包含决策理由和后果

</quality_checks>

<folder_constraints>

## 文件夹约束规则

### 权限和职责

**`docs/requirements/`**: 
- 负责Agent: Requirements Analyst
- 只能创建US和TechSpec

**`docs/sprints/active/`**:
- 负责Agent: Sprint Planning

**`docs/architecture/`**:
- 负责Agent: Architecture Design
- ADR编号连续递增
- 重要决策必须有ADR

**`docs/code-review/`**:
- 负责Agent: Code Review
- 按月组织（YYYY-MM）
- PR编号与Git PR对应

### 禁止操作

❌ 不允许在根目录创建散乱文档  
❌ 不允许创建未定义的子目录  
❌ 不允许跨目录存放错误类型的文档  
❌ 不允许使用非标准文件名格式

</folder_constraints>