---
description: Sprint规划工作流 - 制定可执行的迭代计划
auto_execution_mode: 3
---

# Sprint规划工作流

## 使用场景
在Sprint开始前，将需求分解为可执行的任务并制定迭代计划。

## 前置条件
- [ ] Product Backlog已优先级排序
- [ ] User Story已编写和评审
- [ ] 团队capacity已确定
- [ ] 技术依赖已识别

## Sprint基本信息

**FlyKylin项目Sprint规范**：
- Sprint周期：2周
- Sprint容量：20-30故事点
- Definition of Done：代码审查通过 + 测试覆盖率≥80% + 双平台验证
- 平台：Windows x64 + RK3566 Linux ARM64

## 执行步骤

### 1. Sprint目标设定

```
@cascade 帮我制定Sprint X的目标：

当前迭代：Sprint X (第X个两周迭代)
可用User Stories：
- US-001: [标题]  (优先级: High, 故事点: 5)
- US-002: [标题]  (优先级: Medium, 故事点: 8)
- ...

技术约束：
- 必须兼容Windows和RK3566
- AI功能需考虑性能差异
- 依赖Sprint X-1的成果

请建议：
1. Sprint目标（1-2句话，业务价值导向）
2. 推荐纳入的User Stories
3. Sprint里程碑（可演示的阶段性成果）
4. 风险识别
```

### 2. User Story选择和分解

```
@cascade 评估以下User Stories是否适合本Sprint：

Sprint容量：25故事点
可选Stories：
@docs/requirements/backlog/

评估标准：
- [ ] 故事点总和 ≤ Sprint容量
- [ ] 依赖关系可满足
- [ ] 技术风险可控
- [ ] 符合Sprint目标
- [ ] 可在2周内完成

输出：
1. 建议纳入的Stories（按优先级）
2. 需要拆分的大Story
3. 依赖关系梳理
4. 风险评估
```

### 3. 任务分解

```
@cascade 将User Story分解为开发任务：

User Story: US-XXX
@docs/requirements/sprint_X/user_story_XXX.md

任务分解包括：
1. **设计任务**
   - 接口设计（I_XXX定义）
   - 数据结构设计（Protobuf/数据库）
   - 类图设计

2. **开发任务**
   - 核心逻辑实现
   - 平台特定代码（Windows/RK3566）
   - UI实现
   - 数据持久化

3. **测试任务**
   - 单元测试编写
   - 集成测试
   - 性能测试
   - 跨平台验证

4. **文档任务**
   - API文档
   - 用户文档更新

每个任务估算工时（小时）并分配责任人。
```

### 4. 制定Sprint计划

```
@cascade 生成Sprint计划文档：

Sprint: Sprint X
时间：[开始日期] - [结束日期]
目标：[Sprint目标]

# Sprint X Plan

## Sprint目标
[1-2句话描述业务价值]

## 纳入的User Stories

### US-XXX: [标题] (P0, 5点)
**验收标准**:
- [ ] AC1
- [ ] AC2

**任务分解**:
1. [ ] Task 1 - 设计接口 (4h) @开发者
2. [ ] Task 2 - 实现核心逻辑 (8h) @开发者
3. [ ] Task 3 - 编写单元测试 (4h) @开发者
4. [ ] Task 4 - 双平台验证 (2h) @QA

**依赖**: 无
**风险**: 低

### US-XXX: [标题] (P1, 8点)
...

## Sprint里程碑

**Week 1**:
- Day 3: US-XXX核心功能完成
- Day 5: US-XXX测试通过

**Week 2**:
- Day 8: 所有Stories开发完成
- Day 9: Code Review完成
- Day 10: Sprint Review演示

## 风险和应对

**风险1**: RK3566 NPU性能不足
- 影响: High
- 应对: 准备降级方案（CPU推理）

**风险2**: 第三方库兼容性
- 影响: Medium
- 应对: 提前验证POC

## Definition of Done检查
- [ ] 所有验收标准满足
- [ ] 代码通过Code Review
- [ ] 单元测试覆盖率 ≥ 80%
- [ ] 集成测试通过
- [ ] 文档更新完成
- [ ] Windows和RK3566双平台验证通过

保存到: docs/sprints/sprint_X/plan.md
```

### 5. Daily Standup跟踪

```
@cascade 生成Daily Standup模板：

# Sprint X - Daily Standup

## [日期]

### 开发者A
- **昨天完成**: Task 1 - 接口设计完成
- **今天计划**: Task 2 - 实现核心逻辑
- **阻碍**: 需要确认数据库表结构

### 开发者B
- **昨天完成**: Task 3 - 单元测试50%
- **今天计划**: 完成剩余测试用例
- **阻碍**: 无

## Burndown状态
- 剩余故事点: 18/25
- 预计完成: 按计划

## 风险更新
- RK3566性能问题已解决（降级CPU推理）
```

### 6. Sprint Review准备

```
@cascade 准备Sprint Review演示：

Sprint: Sprint X
演示时间: 30分钟

演示内容：
1. **Sprint目标回顾** (2分钟)
   - 目标: [Sprint目标]
   - 完成情况: X/Y个Stories

2. **功能演示** (20分钟)
   - US-XXX: P2P节点发现
     * 演示: 启动两个实例，自动发现
     * 验收: 5秒内发现，30秒超时移除
   
   - US-XXX: NSFW检测
     * 演示: 发送正常/NSFW图片
     * 验收: 双端检测，警告提示

3. **技术亮点** (5分钟)
   - 跨平台AI加速（DirectML/NPU）
   - 性能达标（Windows 300ms, RK3566 800ms）

4. **下一步计划** (3分钟)
   - Sprint X+1目标
```

## Sprint指标跟踪

### Velocity（速度）
- Sprint 1: 20点
- Sprint 2: 25点
- Sprint 3: 28点
- **目标**: 稳定在25-30点

### Quality（质量）
- 代码覆盖率: ≥ 80%
- Code Review通过率: 100%
- Bug逃逸率: < 5%

### Technical Debt（技术债）
- 每Sprint预留20%时间处理技术债
- 定期重构降低复杂度

## Sprint回顾模板

```
@cascade 生成Sprint回顾：

# Sprint X Retrospective

## 什么做得好？（Keep）
- 双平台持续集成运行流畅
- Code Review质量高，发现XX个问题
- 测试覆盖率达到85%

## 什么需要改进？（Improve）
- RK3566性能测试滞后
- 文档更新不及时

## 行动计划（Action）
1. [ ] 建立RK3566自动化测试环境 (@DevOps, Week 1)
2. [ ] 文档与代码同步更新纳入DoD (@Team, 立即)
3. [ ] 每周五下午技术分享会 (@Team, 持续)

## 数据统计
- 计划故事点: 25
- 完成故事点: 23
- 速度: 92%
- Bug数: 3（已修复2个）
- 测试覆盖率: 85%
```

## 常见Sprint模式

### 功能Sprint
专注于新功能开发：
- 目标：实现X个新功能
- 比例：70%新功能 + 20%测试 + 10%技术债

### 优化Sprint
专注于性能和质量：
- 目标：性能提升X%，技术债减少X%
- 比例：50%优化 + 30%重构 + 20%技术债

### 稳定Sprint  
专注于Bug修复和稳定性：
- 目标：修复所有Critical Bug
- 比例：80%Bug修复 + 20%测试加固

## 注意事项

- **避免过度承诺**：预留缓冲（实际容量80%）
- **考虑跨平台**：Windows和RK3566都要验证
- **性能基线**：明确性能目标（< 500ms / < 1s）
- **依赖管理**：提前识别阻塞任务
- **风险预案**：高风险Story准备Plan B

## 相关文档

- `.windsurf/workflows/requirements-analysis.md` - 需求分析
- `.windsurf/workflows/development-execution.md` - 开发执行
- `.windsurf/rules/project-architecture.md` - 架构约束
- `docs/飞秋方案.md` - 项目整体方案