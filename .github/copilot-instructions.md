# FlyKylin AI Coding Instructions

## Project Overview
FlyKylin is an AI-powered P2P LAN communication tool (modern "Feiq") targeting Windows (x64) and Linux ARM64 (RK3566).
- **Stack**: C++20, Qt 6.5+, CMake, vcpkg, Protobuf, ONNX Runtime, SQLite.
- **Architecture**: Hexagonal/Clean Architecture with MVVM for UI.

## Architecture & Structure
- **Core Logic** (`src/core/`): Platform-independent business logic (P2P discovery, messaging).
- **UI Layer** (`src/ui/`): Qt Widgets/QML. Follows MVVM pattern (`viewmodels/`, `widgets/`).
- **Protocol** (`protocol/`): Protobuf definitions (`messages.proto`) for all network communication.
- **Platform** (`src/platform/`): Hardware abstraction implementations (Windows DirectML vs RK3566 NPU).
- **AI Engine** (`src/ai/`): ONNX Runtime integration for NSFW detection, translation, etc.

## Build & Test Workflow
- **Build System**: CMake with Presets.
  - **Windows**: `cmake --preset windows-release` or `windows-debug`.
  - **Linux (RK3566)**: `cmake --preset linux-arm64-release`.
- **Dependencies**: Managed via `vcpkg.json` (protobuf, sqlite3, gtest). Qt is external (env var `CMAKE_PREFIX_PATH` or hardcoded in presets).
- **Testing**: Google Test. Enable with `-DBUILD_TESTS=ON`.
  - Run tests: `ctest --preset windows-release` (or relevant preset).

## Coding Conventions
- **Naming**:
  - Classes: `PascalCase` (e.g., `PeerDiscovery`).
  - Interfaces: `I_` prefix (e.g., `I_NetworkAdapter`).
  - Members: `m_` prefix + `camelCase` (e.g., `m_socket`).
  - Functions/Variables: `camelCase`.
- **Memory Management**:
  - Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) and RAII.
  - Avoid raw `new`/`delete`.
  - Qt objects: Use parent-child ownership or `QPointer` for weak references.
- **Concurrency**:
  - UI operations **must** be on the main thread.
  - Network/AI operations on background threads.
  - Use Qt Signals/Slots (`Qt::QueuedConnection`) for cross-thread communication.
  - Protect shared data with `std::mutex` or `QMutex`.

## Key Patterns
- **Interfaces**: Decouple layers using abstract interfaces (e.g., `I_MessageSerializer`).
- **Protobuf**: Use generated Protobuf classes for data serialization. Do not manually parse bytes.
- **Platform Abstraction**: Use `#ifdef Q_OS_WIN` or `#ifdef RK3566_PLATFORM` only in `src/platform/` or low-level abstractions. Keep core logic clean.
- **Error Handling**: Use `std::optional` for fallible returns. Log critical errors with `qCritical()`.

## Common Tasks
- **Adding a Message**:
  1. Update `protocol/messages.proto`.
  2. Rebuild to generate C++ code.
  3. Implement handler in `src/core/`.
- **Adding a UI Feature**:
  1. Create ViewModel in `src/ui/viewmodels/`.
  2. Create Widget/Window in `src/ui/widgets/`.
  3. Bind using Qt Signals/Slots.
