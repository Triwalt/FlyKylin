# 多Agent项目迭代工作流

## 概述

本文档定义了基于AI Agent的智能飞秋项目迭代开发工作流，通过专业分工的Agent协作，实现高效、高质量的软件开发流程。

## 工作流架构图

```
┌─────────────────────────────────────────────────────────────┐
│                      项目管理中枢                              │
│                   (Orchestrator Agent)                       │
└─────────────────────────────────────────────────────────────┘
                             │
        ┌────────────────────┼────────────────────┐
        ▼                    ▼                    ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│需求分析Agent  │    │架构设计Agent  │    │Sprint规划    │
│              │───▶│              │───▶│   Agent      │
└──────────────┘    └──────────────┘    └──────────────┘
                                                │
                    ┌───────────────────────────┤
                    ▼                           ▼
            ┌──────────────┐           ┌──────────────┐
            │开发执行Agent  │           │代码审查Agent  │
            │              │◀─────────▶│              │
            └──────────────┘           └──────────────┘
                    │                           │
                    ▼                           ▼
            ┌──────────────┐           ┌──────────────┐
            │测试Agent      │           │质量监控Agent  │
            │              │◀─────────▶│              │
            └──────────────┘           └──────────────┘
                    │                           │
                    └───────────┬───────────────┘
                                ▼
                        ┌──────────────┐
                        │构建部署Agent  │
                        │              │
                        └──────────────┘
```

## Agent职责划分

### 1. Orchestrator Agent (项目管理中枢)

**职责**：
- 统筹整个项目的开发流程
- 协调各个Agent之间的工作
- 监控项目进度和风险
- 决策Sprint优先级和资源分配

**输入**：
- 项目总体需求文档
- 各Agent的工作报告
- Sprint回顾总结

**输出**：
- 项目里程碑计划
- Sprint分配指令
- 风险预警报告
- 资源调配决策

**关键能力**：
- 项目全局视角
- 风险识别和管理
- 决策权衡能力
- 团队协调能力

---

### 2. Requirements Analyst Agent (需求分析Agent)

**职责**：
- 解析和细化项目需求
- 编写用户故事(User Story)
- 定义验收标准
- 需求变更管理

**输入**：
- 原始需求文档
- 用户反馈
- Sprint回顾中的需求变更

**输出**：
- 详细需求规格说明(SRS)
- User Story列表(包含验收标准)
- 需求优先级排序
- 需求追踪矩阵

**工作流程**：
1. 接收原始需求
2. 进行需求澄清(通过问题列表)
3. 拆分为可实现的User Story
4. 定义每个Story的DoD (Definition of Done)
5. 与架构设计Agent协作确认可行性

**输出文件**：
- `docs/requirements/user_stories_sprint_N.md`
- `docs/requirements/acceptance_criteria.md`
- `docs/requirements/requirements_traceability_matrix.xlsx`

---

### 3. Architecture Design Agent (架构设计Agent)

**职责**：
- 设计系统架构
- 制定技术方案
- 定义接口规范
- 技术选型和评估

**输入**：
- 需求规格说明
- 现有架构文档
- 技术约束条件

**输出**：
- 系统架构设计文档
- 接口设计规范(API Doc)
- 数据库设计(ER图、表结构)
- 技术选型报告

**工作流程**：
1. 分析需求的技术特征
2. 评估现有架构的适配性
3. 设计新功能的架构方案
4. 定义模块间接口
5. 评审架构设计(与代码审查Agent协作)

**输出文件**：
- `docs/architecture/system_design_vX.X.md`
- `docs/architecture/api_specification.yaml` (OpenAPI)
- `docs/architecture/database_schema.sql`
- `docs/architecture/tech_stack_decision.md`

---

### 4. Sprint Planning Agent (Sprint规划Agent)

**职责**：
- 规划Sprint任务
- 估算开发工时
- 分配开发任务
- 制定Sprint目标

**输入**：
- User Story列表
- 团队开发能力评估
- 技术方案文档

**输出**：
- Sprint Backlog
- 任务分解WBS (Work Breakdown Structure)
- 开发时间线
- Sprint目标声明

**工作流程**：
1. 从Product Backlog选择Stories
2. 将Stories分解为具体任务
3. 估算每个任务的工时
4. 分配任务到开发Agent
5. 制定Sprint燃尽图基线

**输出文件**：
- `sprints/sprint_N/sprint_backlog.md`
- `sprints/sprint_N/task_breakdown.md`
- `sprints/sprint_N/sprint_goal.md`
- `sprints/sprint_N/timeline.md`

---

### 5. Development Execution Agent (开发执行Agent)

**职责**：
- 编写代码实现
- 编写单元测试
- 本地测试验证
- 提交代码和文档

**输入**：
- 任务描述
- 技术方案
- 接口规范
- 编码规范

**输出**：
- 功能代码
- 单元测试代码
- 代码注释和文档
- Commit记录

**工作流程**：
1. 领取任务
2. 阅读技术方案和接口规范
3. 编写代码和单元测试
4. 本地运行测试
5. 提交到feature分支
6. 发起Pull Request

**输出文件**：
- `src/**/*.cpp`(或其他源文件)
- `tests/**/*_test.cpp`
- Git commit记录
- Pull Request描述

**质量标准**：
- 代码覆盖率 ≥ 80%
- 所有单元测试通过
- 无明显编码规范违反
- 完整的代码注释

---

### 6. Code Review Agent (代码审查Agent)

**职责**：
- 审查代码质量
- 检查编码规范
- 发现潜在缺陷
- 提供改进建议

**输入**：
- Pull Request
- 编码规范文档
- 架构设计文档

**输出**：
- 代码审查报告
- 问题列表(Critical/Major/Minor)
- 改进建议
- 审查通过/拒绝决策

**审查检查点**：
1. **功能正确性**：代码是否实现了需求
2. **架构一致性**：是否符合架构设计
3. **编码规范**：是否遵循C++/Qt编码标准
4. **性能优化**：是否存在性能瓶颈
5. **安全性**：是否存在安全漏洞
6. **可维护性**：代码可读性和可维护性
7. **测试覆盖**：单元测试是否充分

**输出文件**：
- `reviews/pr_XXXX_review.md`
- `reviews/code_quality_metrics.json`
- Pull Request评论

---

### 7. Testing Agent (测试Agent)

**职责**：
- 执行集成测试
- 执行功能测试
- 执行性能测试
- 编写测试报告

**输入**：
- 已合并的代码
- 测试用例
- 验收标准

**输出**：
- 测试报告
- Bug列表
- 测试覆盖率报告
- 性能测试基准

**测试类型**：
1. **集成测试**：测试模块间交互
2. **功能测试**：测试端到端用户场景
3. **性能测试**：测试响应时间、吞吐量等
4. **兼容性测试**：Windows/RK3566跨平台测试
5. **回归测试**：确保新代码未破坏旧功能

**工作流程**：
1. 根据User Story编写测试用例
2. 在测试环境部署应用
3. 执行自动化测试脚本
4. 执行手工测试(UI/UX)
5. 记录Bug并分配给开发Agent
6. 验证Bug修复
7. 生成测试报告

**输出文件**：
- `tests/integration/test_report_sprint_N.md`
- `tests/functional/test_cases.md`
- `tests/performance/benchmark_report.md`
- `bugs/bug_list_sprint_N.md`

---

### 8. Quality Assurance Agent (质量监控Agent)

**职责**：
- 监控代码质量指标
- 跟踪技术债务
- 生成质量报告
- 提出改进建议

**输入**：
- 静态代码分析结果
- 测试覆盖率报告
- Bug统计数据
- 代码复杂度报告

**输出**：
- 质量看板
- 技术债务清单
- 质量趋势报告
- 改进行动计划

**监控指标**：
1. **代码质量**
   - 代码覆盖率
   - 圈复杂度
   - 代码重复率
   - 编码规范违规数

2. **缺陷质量**
   - Bug密度
   - Bug修复时间
   - Bug重开率
   - 严重Bug占比

3. **交付质量**
   - Sprint完成率
   - 功能交付准时率
   - 用户满意度

**工作流程**：
1. 收集各项质量指标
2. 分析质量趋势
3. 识别质量风险
4. 提出改进建议
5. 生成质量周报/月报

**输出文件**：
- `quality/quality_dashboard_weekly.md`
- `quality/technical_debt_backlog.md`
- `quality/improvement_plan.md`

---

### 9. Build & Deployment Agent (构建部署Agent)

**职责**：
- 自动化构建
- 打包发布
- 部署到测试/生产环境
- 管理版本发布

**输入**：
- 已合并的代码(develop/release分支)
- 构建配置
- 部署脚本

**输出**：
- 构建产物(.exe, .deb, AppImage)
- 发布说明(Release Notes)
- 部署日志
- 版本标签

**工作流程**：
1. 触发构建(CI Pipeline)
2. 编译Windows版本(x64)
3. 交叉编译RK3566版本(ARM64)
4. 运行自动化测试
5. 打包生成安装包
6. 部署到测试环境
7. (如果是Release) 发布到生产环境
8. 创建Git Tag和Release Notes

**CI/CD Pipeline**：
```yaml
# .github/workflows/build.yml 示例
name: Build and Test

on:
  push:
    branches: [develop, main]
  pull_request:
    branches: [develop]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup Qt
        uses: jurplel/install-qt-action@v3
      - name: Build
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release
          cmake --build build --config Release
      - name: Test
        run: ctest --test-dir build -C Release
      - name: Package
        run: cpack -C Release

  build-linux-arm64:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup Cross Compile
        run: |
          sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
      - name: Build
        run: |
          cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-linux-gnu.cmake
          cmake --build build
      - name: Package
        run: cpack
```

**输出文件**：
- `build/ai-feiqiu-v1.0.0-windows-x64.exe`
- `build/ai-feiqiu-v1.0.0-linux-arm64.deb`
- `releases/release_notes_v1.0.0.md`
- Git Tag: `v1.0.0`

---

## Agent协作工作流

### Sprint迭代流程

```
第0天：Sprint Planning
├── Orchestrator Agent: 启动Sprint规划会议
├── Requirements Analyst Agent: 呈现User Stories
├── Architecture Design Agent: 评审技术方案
└── Sprint Planning Agent: 制定Sprint Backlog

第1-10天：Sprint执行
├── Development Execution Agent: 开发功能
│   ├── Day 1-3: Core功能开发
│   ├── Day 4-6: 集成和调试
│   └── Day 7-8: 编写测试和文档
├── Code Review Agent: 持续代码审查
│   └── 每日审查Pull Request
├── Testing Agent: 并行测试
│   ├── Day 3+: 开始集成测试
│   └── Day 7+: 功能测试
└── Quality Assurance Agent: 监控质量指标
    └── 每日质量看板更新

第11-12天：Sprint Review & Retrospective
├── Testing Agent: 提交测试报告
├── Build & Deployment Agent: 发布Sprint版本
├── Orchestrator Agent: 主持回顾会议
└── All Agents: 总结经验教训

第13-14天：Sprint准备
├── Requirements Analyst Agent: 整理下个Sprint的需求
├── Architecture Design Agent: 完善技术方案
└── Sprint Planning Agent: 准备下个Sprint Backlog
```

### 典型工作日流程(Daily Workflow)

```
08:00 - 09:00  Daily Standup(虚拟站会)
├── Orchestrator Agent收集各Agent昨日进展
├── 识别阻塞问题
└── 同步今日计划

09:00 - 12:00  开发时段
├── Development Execution Agent: 编码
├── Code Review Agent: 审查昨日PR
└── Testing Agent: 执行测试用例

12:00 - 13:00  午休

13:00 - 17:00  开发时段
├── Development Execution Agent: 继续编码
├── Testing Agent: Bug验证和回归测试
└── Architecture Design Agent: 设计评审

17:00 - 18:00  日报和准备
├── Quality Assurance Agent: 更新质量看板
├── 所有Agent提交日报给Orchestrator
└── Orchestrator Agent: 更新Sprint燃尽图
```

---

## 文档和交付物管理

### 文档结构

```
ai-feiqiu-project/
├── docs/
│   ├── requirements/
│   │   ├── project_requirements.md         # 总需求
│   │   ├── user_stories_sprint_N.md        # User Stories
│   │   └── acceptance_criteria.md          # 验收标准
│   ├── architecture/
│   │   ├── system_design.md                # 系统设计
│   │   ├── api_specification.yaml          # API规范
│   │   └── database_schema.md              # 数据库设计
│   ├── sprints/
│   │   └── sprint_N/
│   │       ├── sprint_backlog.md           # Sprint待办
│   │       ├── sprint_goal.md              # Sprint目标
│   │       └── sprint_retrospective.md     # Sprint回顾
│   ├── quality/
│   │   ├── quality_dashboard.md            # 质量看板
│   │   └── technical_debt_backlog.md       # 技术债务
│   └── releases/
│       └── release_notes_vX.X.X.md         # 发布说明
├── agents/
│   ├── agent_prompts.yaml                  # Agent提示词配置
│   └── agent_workflow.md                   # 本文档
├── src/                                     # 源代码
├── tests/                                   # 测试代码
└── build/                                   # 构建产物
```

---

## Agent配置和调用

每个Agent都有专门的提示词(Prompt)配置，存储在`agents/agent_prompts.yaml`中。
具体配置见该文件。

在实际工作中，可以通过以下方式调用Agent：

```python
# 伪代码示例
orchestrator = Agent("Orchestrator", prompt=load_prompt("orchestrator"))
req_analyst = Agent("Requirements Analyst", prompt=load_prompt("requirements_analyst"))
# ...

# Sprint Planning
sprint_plan = orchestrator.plan_sprint(
    user_stories=req_analyst.analyze_requirements(raw_requirements),
    architecture=arch_agent.design_system(user_stories),
    team_capacity=get_team_capacity()
)

# Development
for task in sprint_plan.tasks:
    code = dev_agent.implement(task)
    review_result = code_review_agent.review(code)
    if review_result.approved:
        merge(code)
        test_agent.test(code)
```

---

## 质量保证机制

1. **代码审查门禁**：所有代码必须通过Code Review Agent审查才能合并
2. **测试覆盖率要求**：单元测试覆盖率≥80%，集成测试覆盖核心功能
3. **持续集成**：每次提交自动运行CI Pipeline
4. **质量看板**：每日更新质量指标，及时发现问题
5. **技术债务管理**：定期review技术债务，安排还债Sprint

---

## 风险管理

| 风险类型 | 负责Agent | 应对策略 |
|---------|----------|---------|
| 需求不明确 | Requirements Analyst | 早期需求澄清，频繁沟通 |
| 技术难点 | Architecture Design | 技术预研，原型验证 |
| 进度延期 | Orchestrator | 调整Sprint范围，增加资源 |
| 质量下降 | Quality Assurance | 提高代码审查标准，加强测试 |
| NPU适配困难 | Development Execution | 提前在RK3566上验证，准备CPU fallback方案 |

---

## 附录：Agent Prompt设计原则

1. **角色明确**：清晰定义Agent的职责边界
2. **输入输出规范**：明确输入格式和输出格式
3. **质量标准**：提供可量化的质量指标
4. **协作协议**：定义与其他Agent的交互方式
5. **持续改进**：根据实际效果迭代Prompt

具体Prompt配置见`agents/agent_prompts.yaml`。
