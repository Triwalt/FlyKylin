---
description: 需求分析工作流 - 从原始需求到User Story
auto_execution_mode: 3
---

# 需求分析工作流

## 使用场景
当你需要将原始需求转化为可执行的User Story时使用此工作流。

## 前置条件
- [ ] 已有初步需求描述
- [ ] 明确目标用户
- [ ] 了解技术约束（C++20/Qt6、跨平台、RK3566性能限制）

## 执行步骤

### 1. 需求澄清

```
@cascade 我需要分析以下需求：

[粘贴原始需求]

请帮我：
1. 识别需求中的模糊点和潜在歧义
2. 列出需要澄清的问题（至少3个）
3. 分析技术可行性（考虑C++20/Qt6技术栈和RK3566性能）
4. 识别潜在风险和依赖
```

### 2. 编写User Story

```
@cascade 基于澄清后的需求，请编写User Story。

遵循INVEST原则：
- Independent: 独立可交付
- Negotiable: 可讨论调整
- Valuable: 用户价值明确
- Estimable: 可估算工作量
- Small: 1个Sprint内完成
- Testable: 可验证

输出格式：
**标题**: US-XXX [简短描述]

**User Story**:
作为 [用户角色]
我希望 [功能描述]
以便 [业务价值]

**验收标准** (Given-When-Then):
1. Given [前置条件] When [操作] Then [预期结果]
2. Given [前置条件] When [操作] Then [预期结果]

**技术约束**:
- 平台: Windows x64 / RK3566 Linux
- 性能: [具体指标]
- 依赖: [依赖的模块或Story]

**优先级**: Critical/High/Medium/Low
**故事点**: 1-13点
**Sprint**: Sprint X

保存到: docs/requirements/sprint_X/user_story_XXX.md
```

### 3. 需求分解（如果Story过大）

```
@cascade 这个User Story过大（>8点），请帮我分解：

[粘贴User Story]

分解原则：
- 每个子Story 2-5点
- 保持用户价值完整
- 可独立交付和测试
- 明确先后依赖关系
```

### 4. 编写技术规格

```
@cascade 为以下User Story编写技术规格：

[粘贴User Story]

技术规格包括：
1. **涉及模块**: core/ai/platform/ui/storage
2. **接口设计**: 主要类和接口（使用I_前缀）
3. **数据结构**: Protobuf消息定义或数据库表
4. **算法选择**: 关键算法和实现思路
5. **性能目标**: 响应时间、内存占用
6. **错误处理**: 异常场景和处理策略
7. **测试策略**: 单元测试和集成测试计划

保存到: docs/requirements/sprint_X/tech_spec_XXX.md
```

### 5. 评审检查清单

在完成需求分析后，检查以下项目：

- [ ] User Story符合INVEST原则
- [ ] 验收标准具体可测试
- [ ] 技术可行性已验证
- [ ] 性能指标明确（Windows < 500ms，RK3566 < 1s）
- [ ] 跨平台兼容性已考虑
- [ ] 依赖关系已识别
- [ ] 风险已评估
- [ ] 故事点估算合理（1个Sprint = 20-30点）

## 输出交付物

1. **User Story文档**: `docs/requirements/sprint_X/user_story_XXX.md`
2. **技术规格文档**: `docs/requirements/sprint_X/tech_spec_XXX.md`
3. **需求澄清记录**: 保存在User Story文档中

## 示例

### 示例需求
"我希望在发送图片时自动检测是否包含不适内容"

### 示例User Story

**标题**: US-005 图片NSFW自动检测

**User Story**:
作为 企业员工
我希望 在发送图片前自动检测NSFW内容并拦截
以便 避免发送不当内容，维护良好的工作环境

**验收标准**:
1. Given 用户选择一张图片准备发送 When 图片包含NSFW内容 Then 弹出警告提示"图片包含不适内容，无法发送"
2. Given 用户选择一张图片准备发送 When 图片不包含NSFW内容 Then 正常发送
3. Given 接收方收到图片 When 图片包含NSFW内容 Then 自动模糊处理并显示警告

**技术约束**:
- 平台: Windows (DirectML) / RK3566 (NPU)
- 性能: Windows < 500ms, RK3566 < 1s
- 模型: MobileNetV3 NSFW分类器
- 依赖: AI Engine模块

**优先级**: High
**故事点**: 5
**Sprint**: Sprint 5

## 注意事项

- 需求分析时始终考虑**用户价值**，不要陷入技术细节
- RK3566平台性能有限，需求设计时**预留性能余量**
- 跨平台需求必须在两个平台都可实现
- 涉及AI功能时，明确**模型选择**和**推理性能**要求
- 复杂需求优先**MVP（最小可行产品）**，后续迭代增强

## 相关文档

- `docs/飞秋方案.md` - 项目整体方案
- `.windsurf/rules/project-architecture.md` - 架构约束
- `.windsurf/workflows/sprint-planning.md` - Sprint规划工作流