# Requirements Document

## Introduction

本需求文档定义了FlyKylin项目文档全面更新的范围和验收标准。项目当前已实现大量功能，但文档（特别是README.md）存在过时信息，需要与实际代码状态同步。

## Glossary

- **FlyKylin**: 智能飞秋项目，一个跨平台局域网P2P即时通讯工具
- **NSFW检测**: Not Safe For Work内容检测，使用AI模型识别不适当图片
- **语义搜索**: 基于文本向量化的聊天记录搜索功能
- **RK3566**: Rockchip ARM64嵌入式平台，项目的目标部署平台之一
- **RKNN**: Rockchip Neural Network，RK3566平台的NPU推理框架
- **ONNX Runtime**: 跨平台AI模型推理引擎
- **QML**: Qt Meta Language，Qt的声明式UI框架
- **Protobuf**: Protocol Buffers，Google的序列化协议

## Requirements

### Requirement 1

**User Story:** As a developer, I want the README.md to accurately reflect the current project status, so that I can quickly understand what features are implemented and how to build the project.

#### Acceptance Criteria

1. WHEN a developer reads the README.md THEN the system documentation SHALL display the current implementation status including: QML UI (已启用), TCP/UDP通信 (已实现), Protobuf序列化 (已实现), AI功能 (NSFW检测和语义搜索已实现)
2. WHEN a developer follows the build instructions THEN the system documentation SHALL provide accurate CMake preset commands that work on Windows
3. WHEN a developer reviews the architecture section THEN the system documentation SHALL show the actual implemented architecture with core modules (communication, ai, services, database)
4. WHEN a developer checks the feature list THEN the system documentation SHALL mark completed features with ✅ and in-progress features with 🔄

### Requirement 2

**User Story:** As a developer, I want the project structure documentation to match the actual codebase, so that I can navigate the code efficiently.

#### Acceptance Criteria

1. WHEN a developer reads the project structure THEN the system documentation SHALL list all major directories including: src/core/communication, src/core/ai, src/core/services, src/core/database, src/ui/qml, src/ui/viewmodels
2. WHEN a developer looks for a specific module THEN the system documentation SHALL describe the purpose of each core module accurately
3. WHEN a developer reviews the technology stack THEN the system documentation SHALL list actual dependencies: Qt6, ONNX Runtime, Protobuf, SQLite, QCoro

### Requirement 3

**User Story:** As a developer, I want clear deployment documentation, so that I can deploy the application to both Windows and RK3566 platforms.

#### Acceptance Criteria

1. WHEN a developer deploys to Windows THEN the system documentation SHALL provide working build commands using CMake presets
2. WHEN a developer deploys to RK3566 THEN the system documentation SHALL reference the existing DEPLOYMENT_GUIDE.md with accurate cross-compilation instructions
3. WHEN a developer needs AI models THEN the system documentation SHALL list the required model files and their locations (model/onnx/, model/rknn/)

### Requirement 4

**User Story:** As a developer, I want the Sprint status to reflect actual progress, so that I can understand what has been completed and what remains.

#### Acceptance Criteria

1. WHEN a developer reviews Sprint status THEN the system documentation SHALL show Sprint 1-4 as completed (P2P通信, 用户体系, 群聊文件, 跨平台)
2. WHEN a developer reviews Sprint 5 status THEN the system documentation SHALL show NSFW检测 as implemented with both ONNX and RKNN backends
3. WHEN a developer reviews Sprint 6 status THEN the system documentation SHALL show 语义搜索 as implemented with BGE模型

### Requirement 5

**User Story:** As a developer, I want accurate technical specifications, so that I can understand the system's capabilities and limitations.

#### Acceptance Criteria

1. WHEN a developer reviews the C++ standard THEN the system documentation SHALL state C++17 (not C++20 as previously claimed, based on CMakeLists.txt)
2. WHEN a developer reviews Qt version THEN the system documentation SHALL state Qt6 for Windows and Qt5.12 for RK3566
3. WHEN a developer reviews AI capabilities THEN the system documentation SHALL describe NSFW detection (open_nsfw model) and text embedding (BGE-small-zh-v1.5)

### Requirement 6

**User Story:** As a developer, I want the documentation to remove outdated or misleading information, so that I don't waste time on deprecated approaches.

#### Acceptance Criteria

1. WHEN a developer reads the README THEN the system documentation SHALL NOT reference non-existent files like 飞秋方案.md, 多Agent项目迭代工作流.md, agent_prompts.yaml in the root directory
2. WHEN a developer reads about architecture THEN the system documentation SHALL clarify that ARCHITECTURE_PROPOSAL.md is a future proposal, not current implementation
3. WHEN a developer reads about the Multi-Agent workflow THEN the system documentation SHALL either update or remove references to the 9-agent development system if not actively used

