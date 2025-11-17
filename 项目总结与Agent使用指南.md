# 智能飞秋项目 - 多Agent开发体系总结

## 📋 项目概述

本文档总结了基于多Agent协作的智能飞秋项目开发体系，包含完整的技术方案、开发流程和Agent配置。

## 📁 交付物清单

### 1. 核心文档

| 文档名称 | 路径 | 说明 |
|---------|------|------|
| 飞秋方案.md | `./飞秋方案.md` | 完整的技术方案文档，包含架构设计、技术选型、数据库设计等 |
| 多Agent项目迭代工作流.md | `./多Agent项目迭代工作流.md` | Agent协作流程、职责划分、工作流程 |
| agent_prompts.yaml | `./agent_prompts.yaml` | 9个Agent的详细提示词配置 |
| 本文档 | `./项目总结与Agent使用指南.md` | 使用指南和总结 |

### 2. 文档内容概览

#### 飞秋方案.md
- **项目定位**：下一代智能局域网协作套件
- **核心功能**：P2P通信、群聊、文件传输
- **核心创新**：AI功能(NSFW检测、聊天摘要、语义搜索、消息翻译) + NPU/GPU加速
- **技术栈**：C++20 + Qt6 + ONNX Runtime + RKNPU + SQLite + Protobuf
- **架构设计**：MVVM模式、硬件抽象层(HAL)、模块化设计
- **迭代计划**：6个Sprint，从核心通信到完整AI功能

#### 多Agent项目迭代工作流.md
- **9个专业Agent**：Orchestrator、需求分析、架构设计、Sprint规划、开发执行、代码审查、测试、质量监控、构建部署
- **协作流程**：Sprint迭代流程、每日工作流
- **质量保证**：代码审查门禁、测试覆盖率要求、持续集成
- **文档管理**：清晰的文档结构和交付物规范

#### agent_prompts.yaml
- 每个Agent的角色定义
- 详细的系统提示词(system_prompt)
- 任务模板(task_prompt_template)
- 质量标准和输出要求

## 🚀 快速开始指南

### Step 1: 理解项目架构

1. **阅读技术方案**
   ```bash
   打开 "飞秋方案.md"
   重点阅读：
   - 第1章：项目定位与挑战
   - 第3章：软件架构设计
   - 第4章：通信协议设计
   ```

2. **理解AI加速机制**
   - NPU (RK3566): 使用rknn-toolkit2，离线转换ONNX到.rknn模型
   - GPU (Windows): 使用ONNX Runtime + DirectML/CUDA
   - CPU Fallback: 所有平台通用备份方案
   - 硬件抽象层：`I_Accelerator`接口统一调用

### Step 2: 配置Agent环境

1. **加载Agent配置**
   ```python
   import yaml
   
   # 加载Agent配置
   with open('agent_prompts.yaml', 'r', encoding='utf-8') as f:
       agent_config = yaml.safe_load(f)
   
   # 获取特定Agent的配置
   orchestrator_prompt = agent_config['orchestrator_agent']['system_prompt']
   ```

2. **初始化Agent**
   ```python
   # 伪代码示例
   from ai_agent import Agent
   
   # 创建9个Agent实例
   orchestrator = Agent(
       name="Orchestrator",
       system_prompt=agent_config['orchestrator_agent']['system_prompt']
   )
   
   req_analyst = Agent(
       name="Requirements Analyst",
       system_prompt=agent_config['requirements_analyst_agent']['system_prompt']
   )
   
   # ... 创建其他7个Agent
   ```

### Step 3: 启动第一个Sprint

1. **需求分析**
   ```python
   # 使用Requirements Analyst Agent分析需求
   user_stories = req_analyst.analyze_requirements(
       raw_requirements="""
       需求：实现局域网P2P自动发现和即时通讯
       - 支持UDP广播发现在线用户
       - 建立TCP长连接进行聊天
       - 支持文本和文件传输
       """
   )
   ```

2. **架构设计**
   ```python
   # 使用Architecture Design Agent设计系统
   architecture = arch_agent.design_system(
       user_stories=user_stories,
       existing_architecture="现有架构文档路径"
   )
   ```

3. **Sprint规划**
   ```python
   # 使用Sprint Planning Agent制定计划
   sprint_backlog = sprint_planning_agent.plan_sprint(
       user_stories=user_stories,
       architecture=architecture,
       team_capacity={"agent_hours": 80}  # 一个Sprint的总工时
   )
   ```

4. **开发执行**
   ```python
   # 使用Development Execution Agent开发
   for task in sprint_backlog.tasks:
       code_result = dev_agent.implement(
           task_description=task.description,
           architecture_design=architecture,
           api_specification=task.api_spec
       )
       
       # 代码审查
       review_result = code_review_agent.review(
           code_diff=code_result.diff,
           architecture_design=architecture
       )
       
       if review_result.approved:
           # 合并代码
           merge_pr(code_result.pr_id)
           
           # 测试
           test_result = testing_agent.test(
               user_story=task.user_story,
               code_changes=code_result
           )
   ```

### Step 4: 质量监控与发布

1. **质量监控**
   ```python
   # 使用Quality Assurance Agent监控质量
   quality_report = qa_agent.monitor_quality(
       static_analysis=run_static_analysis(),
       coverage_report=get_coverage_report(),
       bug_statistics=get_bug_stats(),
       sprint_progress=get_sprint_progress()
   )
   ```

2. **构建部署**
   ```python
   # 使用Build & Deployment Agent构建和部署
   build_result = build_agent.build_and_deploy(
       branch="develop",
       target_platforms=["Windows x64", "Linux ARM64"],
       build_type="Release"
   )
   ```

## 📊 Agent协作示例

### 示例1：实现NSFW检测功能

```
第1步：需求分析 (Requirements Analyst Agent)
输出：
User Story: 作为用户，我希望在发送图片前自动检测NSFW内容，以便避免发送不当图片
验收标准：
- Given 用户选择图片发送
- When 图片包含NSFW内容
- Then 显示警告并阻止发送

第2步：架构设计 (Architecture Design Agent)
输出：
- 模块：AIEngine, I_Accelerator, Rk3566NpuEngine
- 接口：bool CheckNsfw(const QByteArray& imageData)
- 双端检测：发送端检测 + 接收端检测
- NPU资源管理：使用互斥锁管理模型切换

第3步：Sprint规划 (Sprint Planning Agent)
输出：
任务1: 实现I_Accelerator接口 (4h)
任务2: 实现Rk3566NpuEngine (8h)
任务3: 实现AIEngine资源管理 (4h)
任务4: 集成到ChatViewModel (4h)
任务5: 编写单元测试 (4h)

第4步：开发执行 (Development Execution Agent)
输出：
- src/core/ai/i_accelerator.h
- src/platform/rk3566/npu_engine_rknn.cpp
- src/core/ai/ai_engine.cpp
- tests/platform/test_npu_engine.cpp

第5步：代码审查 (Code Review Agent)
输出：
总体评价: Request Changes
问题：
- [Major] NPU资源管理中未处理rknn_init失败的情况
- [Minor] 建议增加性能日志
改进建议：
- 在rknn_init失败时，回退到CpuFallbackEngine
- 添加模型切换耗时的性能监控

第6步：测试 (Testing Agent)
输出：
测试用例：
- 测试NSFW图片检测准确率
- 测试安全图片不被误判
- 测试NPU资源管理并发安全
- 测试性能(检测耗时<100ms)

第7步：质量监控 (Quality Assurance Agent)
输出：
- 代码覆盖率: 85% ✅
- 圈复杂度: 8 ✅
- Bug密度: 0/KLOC ✅
- 性能测试: NSFW检测平均45ms ✅

第8步：构建部署 (Build & Deployment Agent)
输出：
- Windows版本: 编译成功
- RK3566版本: 编译成功，NPU模型加载正常
- 部署到测试环境: 成功
```

## 🎯 关键设计决策

### 1. 多Agent协作 vs 单一AI助手

**选择多Agent的原因：**
- ✅ **专业分工**：每个Agent专注一个领域，提高专业度
- ✅ **并行工作**：开发、测试、代码审查可并行进行
- ✅ **质量保证**：代码审查Agent和质量监控Agent提供双重保障
- ✅ **可扩展性**：轻松增加新的Agent(如安全扫描Agent)
- ✅ **责任清晰**：每个环节有明确的负责Agent

### 2. 基于Prompt的Agent vs 微调模型

**选择Prompt-based的原因：**
- ✅ **灵活性**：通过YAML配置即可调整Agent行为
- ✅ **快速迭代**：不需要重新训练模型
- ✅ **成本低**：无需大量标注数据和训练成本
- ✅ **可解释性**：Prompt内容清晰，行为可预测
- ✅ **易于维护**：修改Prompt比重新训练模型容易得多

### 3. Sprint-based vs Kanban

**选择Sprint-based敏捷开发的原因：**
- ✅ **明确节奏**：2周一个Sprint，节奏清晰
- ✅ **可预测性**：Sprint规划提供可预测的交付
- ✅ **回顾机制**：Sprint回顾促进持续改进
- ✅ **风险控制**：固定时间盒，及时发现和处理风险

## 🔍 技术亮点

### 1. 硬件抽象层(HAL)设计

```cpp
// 统一的AI加速接口
class I_Accelerator {
public:
    virtual bool Initialize() = 0;
    virtual bool RunNsfwCheck(const void* imageData, size_t size) = 0;
    virtual std::vector<float> RunEmbedding(const std::string& text) = 0;
    virtual std::string RunSummary(const std::string& longText) = 0;
};

// 平台特定实现自动选择
std::unique_ptr<I_Accelerator> CreateAccelerator() {
#ifdef RK3566_PLATFORM
    auto npu = std::make_unique<Rk3566NpuEngine>();
    if (npu->Initialize()) return npu;
#elif defined(WINDOWS_PLATFORM)
    auto gpu = std::make_unique<WindowsGpuEngine>();
    if (gpu->Initialize()) return gpu;
#endif
    return std::make_unique<CpuFallbackEngine>();
}
```

**优势：**
- 上层代码无需关心硬件差异
- 自动回退机制保证可用性
- 易于扩展新的硬件平台

### 2. AI协同机制

**场景**：低性能设备请求高性能设备执行AI计算

```protobuf
// 协议定义
message AiSummaryRequest {
  string long_text_content = 1;
}

message AiSummaryResponse {
  string original_hash = 1;
  string summary_text = 2;
}
```

**流程：**
1. RK3566检测到聊天摘要请求
2. 查找局域网内GPU节点
3. 发送`MSG_AI_SUMMARY_REQUEST`
4. GPU节点执行摘要并返回结果
5. 如无GPU节点，本地NPU执行

**优势：**
- 充分利用局域网资源
- 低功耗设备也能享受高级AI功能
- 优雅降级保证功能可用

### 3. NPU资源管理

**挑战**：RK3566 NPU内存小(0.8 TOPS)，无法同时加载多个模型

**解决方案**：动态模型切换 + 互斥锁

```cpp
class AIEngine {
private:
    std::mutex npu_lock_;
    AiTask current_model_ = AiTask::None;
    
public:
    bool CheckNsfw(const QByteArray& imageData) {
        std::lock_guard<std::mutex> lock(npu_lock_);
        
        if (current_model_ != AiTask::NsfwDetection) {
            // 卸载旧模型，加载NSFW模型
            rknn_release(current_context_);
            rknn_init(&current_context_, nsfw_model_path_);
            current_model_ = AiTask::NsfwDetection;
        }
        
        // 执行推理
        return RunInference(imageData);
    }
};
```

**优势：**
- 有效管理有限的NPU资源
- 多个AI功能共享同一个NPU
- 线程安全

## 📈 质量保证体系

### 1. 代码质量门禁

| 检查项 | 标准 | 工具 |
|-------|------|------|
| 代码覆盖率 | ≥80% | lcov/gcov |
| 圈复杂度 | <10 | cppcheck |
| 代码重复率 | <5% | CPD |
| 编码规范 | 0违规 | clang-tidy |
| 单元测试 | 全部通过 | GoogleTest |

### 2. 持续集成流程

```yaml
# GitHub Actions Pipeline
on: [push, pull_request]

jobs:
  build-and-test:
    steps:
      - Checkout代码
      - 安装依赖(Qt, ONNX Runtime)
      - 编译(Windows + Linux ARM64)
      - 运行单元测试
      - 静态代码分析
      - 生成覆盖率报告
      - 代码质量检查
      - 构建产物上传
```

### 3. 测试金字塔

```
        /\
       /  \    E2E Tests (10%)
      /----\   
     /      \  Integration Tests (20%)
    /--------\
   /          \ Unit Tests (70%)
  /____________\
```

- **单元测试70%**：快速、稳定、易维护
- **集成测试20%**：测试模块间交互
- **端到端测试10%**：测试关键用户场景

## 🛠️ 开发工具链

### 必需工具

| 工具 | 版本 | 用途 |
|-----|------|------|
| C++ Compiler | C++20 | 编译器(MSVC/GCC) |
| Qt | 6.x | UI框架 |
| CMake | 3.20+ | 构建系统 |
| Git | 2.x | 版本控制 |
| ONNX Runtime | 1.14+ | AI推理 |
| rknn-toolkit2 | 1.5+ | RK3566 NPU |

### 推荐工具

| 工具 | 用途 |
|-----|------|
| CLion / VS Code | IDE |
| clang-format | 代码格式化 |
| cppcheck | 静态分析 |
| Valgrind / Dr. Memory | 内存检测 |
| Qt Creator | Qt开发 |

## 📚 学习路径

### 初学者路径

1. **Week 1-2: 理解基础**
   - 学习C++20基础特性
   - 学习Qt 6基础(Signal/Slot, QML)
   - 理解MVVM架构模式

2. **Week 3-4: P2P通信**
   - 学习UDP广播和TCP长连接
   - 学习Protobuf协议
   - 实现简单的P2P聊天Demo

3. **Week 5-6: AI集成**
   - 学习ONNX Runtime使用
   - 理解硬件抽象层设计
   - 在CPU上运行NSFW模型

4. **Week 7-8: 跨平台**
   - 学习CMake跨平台构建
   - 学习RK3566 NPU开发
   - 实现NPU加速的Demo

### 进阶路径

1. **架构设计**
   - 深入学习SOLID原则
   - 学习设计模式(Factory, Strategy, Observer等)
   - 实践领域驱动设计(DDD)

2. **性能优化**
   - 学习C++性能分析工具(perf, vtune)
   - 学习多线程和异步编程
   - 学习SIMD优化

3. **AI模型优化**
   - 学习模型量化(INT8/FP16)
   - 学习模型剪枝
   - 学习模型蒸馏

## 🐛 常见问题与解决方案

### Q1: NPU模型转换失败

**问题**：ONNX模型无法转换为.rknn格式

**解决方案**：
1. 检查模型算子是否被RKNPU支持
2. 使用rknn-toolkit2的`onnx2rknn`工具，添加`-v`参数查看详细日志
3. 尝试简化模型或使用CPU fallback

### Q2: UDP广播无法发现对方

**问题**：在WiFi环境下无法发现其他节点

**解决方案**：
1. 检查防火墙是否阻止UDP端口
2. 尝试使用mDNS作为备选发现机制
3. 确认在同一个子网内
4. 增加广播频率(从5秒减少到2秒)

### Q3: Qt跨平台编译问题

**问题**：Windows上编译通过，ARM64上失败

**解决方案**：
1. 确保使用Qt的跨平台API
2. 避免使用Windows特定的API(如`<windows.h>`)
3. 使用CMake的platform check：
   ```cmake
   if(WIN32)
       # Windows specific code
   elseif(UNIX AND NOT APPLE)
       # Linux specific code
   endif()
   ```

### Q4: Agent协作效率低

**问题**：Agent之间传递信息效率低，重复工作多

**解决方案**：
1. 优化Prompt，减少冗余信息
2. 建立共享的知识库(如架构文档)
3. 使用Orchestrator Agent统一协调
4. 实施Agent工作日志，避免重复询问

## 📖 参考资料

### 官方文档

- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [ONNX Runtime Documentation](https://onnxruntime.ai/docs/)
- [Rockchip RKNPU Documentation](https://github.com/rockchip-linux/rknn-toolkit2)
- [Protocol Buffers Guide](https://developers.google.com/protocol-buffers)

### 学习资源

- **C++**: Effective Modern C++ (Scott Meyers)
- **Qt**: Qt 6 C++ GUI Programming Cookbook
- **AI**: Hands-On Machine Learning with Scikit-Learn (模型理解)
- **敏捷开发**: Scrum Guide (官方指南)

### 社区资源

- Stack Overflow: C++/Qt问题
- GitHub: 开源项目参考
- Qt Forum: Qt社区
- ONNX Community: AI模型讨论

## 🎉 总结

本项目通过**多Agent协作**的创新方式，构建了一个：

✅ **专业化**：9个专业Agent各司其职
✅ **自动化**：从需求到部署全流程自动化
✅ **高质量**：多层质量保证机制
✅ **可扩展**：模块化设计易于扩展
✅ **跨平台**：Windows/RK3566无缝支持
✅ **AI驱动**：NPU/GPU加速的智能功能

的新一代局域网协作套件开发体系。

### 核心价值

1. **效率提升**：Agent自动化减少人工工作量
2. **质量保证**：多Agent审查确保代码质量
3. **知识沉淀**：完整的文档和配置沉淀知识
4. **快速迭代**：基于Prompt的Agent易于调整优化

### 下一步行动

1. **立即开始**：按照本指南启动第一个Sprint
2. **持续优化**：根据实际效果迭代Agent配置
3. **分享经验**：记录遇到的问题和解决方案
4. **社区贡献**：将优秀实践反馈到开源社区

---

**祝开发顺利！如有问题，请参考本文档或查阅相关技术文档。** 🚀
