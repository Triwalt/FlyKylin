# Developer Tools

Tools for FlyKylin development environment setup and verification.

## Scripts

### start-vsdevcmd.ps1 ⭐
**Launches VS Developer Command Prompt** - Solves MSVC not found issue.

```powershell
.\tools\developer\start-vsdevcmd.ps1
```

**What it does**:
- Automatically finds Visual Studio 2022 installation
- Opens a new VS Developer Command Prompt window
- Sets up MSVC compiler environment

**Use this if you see**: `[ERROR] MSVC not found!`

### configure-environment.ps1 / .cmd
Configures CMake environment variables (Qt, ONNX paths).

```powershell
# In PowerShell
.\tools\developer\configure-environment.ps1

# In CMD (VS Dev Prompt) - use .cmd wrapper
.\tools\developer\configure-environment.cmd
```

**What it does**:
- Sets `CMAKE_PREFIX_PATH` for Qt
- Sets `ONNXRUNTIME_DIR` for ONNX Runtime
- Verifies all dependencies

### verify-environment.ps1 / .cmd
Checks all required dependencies and tools.

```powershell
# In PowerShell
.\tools\developer\verify-environment.ps1

# In CMD (VS Dev Prompt) - use .cmd wrapper
.\tools\developer\verify-environment.cmd
```

**Checks**:
- Qt 6.9.3 MSVC 2022 64-bit
- ONNX Runtime C++ API
- Protobuf compiler
- CMake
- MSVC compiler
- AI models (NSFW, BGE)

**Note**: VS Developer Command Prompt is CMD, not PowerShell. Use `.cmd` wrappers!

### install-onnx-runtime.ps1
Automatically downloads and installs ONNX Runtime C++ API (v1.23.2).

```powershell
.\tools\developer\install-onnx-runtime.ps1
```

**What it does**:
- Downloads `onnxruntime-win-x64-1.23.2.zip` (about 30MB)
- Extracts to `e:\Project\FlyKylin\3rdparty\onnxruntime`
- Verifies installation

### install-protobuf.ps1
Installs Protobuf compiler using Chocolatey.

```powershell
# Run as Administrator
.\tools\developer\install-protobuf.ps1
```

**Requirements**:
- Chocolatey package manager
- Administrator privileges

**Alternative methods**:
- vcpkg: `.\vcpkg\vcpkg install protobuf:x64-windows`
- Manual: https://github.com/protocolbuffers/protobuf/releases

## Quick Start

**Step 1: Install dependencies**

```powershell
.\tools\developer\install-onnx-runtime.ps1
.\tools\developer\install-protobuf.ps1
```

**Step 2: Launch VS Developer Command Prompt** (solves MSVC issue)

```powershell
.\tools\developer\start-vsdevcmd.ps1
```

**Step 3: In the new VS Dev Prompt window**

```cmd
cd e:\Project\FlyKylin

REM Option 1: Use .cmd wrapper (easier)
.\tools\developer\configure-environment.cmd
.\tools\developer\verify-environment.cmd

REM Option 2: Call PowerShell explicitly
REM powershell -ExecutionPolicy Bypass -File .\tools\developer\configure-environment.ps1
REM powershell -ExecutionPolicy Bypass -File .\tools\developer\verify-environment.ps1

mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

## Troubleshooting

### MSVC not found
**Solution**: Run `.\tools\developer\start-vsdevcmd.ps1` to launch VS Developer Command Prompt

### ONNX Runtime download fails
**Solution**: Manually download from https://github.com/microsoft/onnxruntime/releases/tag/v1.23.2
Extract to `e:\Project\FlyKylin\3rdparty\onnxruntime`

### Protobuf installation fails
**Alternative**: Use vcpkg or manual installation (see script output for instructions)

## Notes

- All scripts use English to avoid PowerShell encoding issues
- Scripts are idempotent (safe to run multiple times)
- Always run build commands in VS Developer Command Prompt
