# FlyKylin Getting Started Guide

Quick guide to set up FlyKylin development environment on Windows.

## Prerequisites

- Windows 10/11
- Visual Studio 2022 with C++ Desktop Development workload
- Internet connection (for downloading dependencies)

## Quick Start (3 Steps)

### Step 1: Install Dependencies

```powershell
# Install ONNX Runtime C++ API
.\tools\developer\install-onnx-runtime.ps1

# Install Protobuf compiler (requires admin)
.\tools\developer\install-protobuf.ps1
```

### Step 2: Launch VS Developer Environment ⭐

```powershell
# Auto-detects VS 2022 and opens Developer Command Prompt
.\tools\developer\start-vsdevcmd.ps1
```

**This solves the "MSVC not found" error!**

### Step 3: Build in VS Developer Prompt

In the newly opened CMD window:

```cmd
cd e:\Project\FlyKylin

REM Configure environment (use .cmd wrapper)
.\tools\developer\configure-environment.cmd

REM Verify all dependencies
.\tools\developer\verify-environment.cmd

REM Build project
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

**Important**: Use `.cmd` files (not `.ps1`) in VS Developer Command Prompt!

## Troubleshooting

### MSVC Compiler Not Found

**Error**: `[ERROR] MSVC not found!`

**Solution**: Always use VS Developer Command Prompt
- Run: `.\tools\developer\start-vsdevcmd.ps1`
- Or manually: Start Menu → "Developer Command Prompt for VS 2022"

### ONNX Runtime Download Fails

**Error**: Network timeout or SSL error

**Solution**: Manual download
1. Visit: https://github.com/microsoft/onnxruntime/releases/tag/v1.23.2
2. Download: `onnxruntime-win-x64-1.23.2.zip` (30MB)
3. Extract to: `e:\Project\FlyKylin\3rdparty\onnxruntime`

### Protobuf Not Found

**Solution 1**: Install Chocolatey first
```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
choco install protoc -y
```

**Solution 2**: Use vcpkg
```powershell
vcpkg install protobuf:x64-windows
```

## Environment Check

Verify all dependencies are installed:

```powershell
.\tools\developer\verify-environment.ps1
```

Expected output:
```
[OK] Qt 6.9.3 MSVC 2022 64-bit
[OK] ONNX Runtime C++ API
[OK] Protobuf: libprotoc 3.x.x
[OK] CMake: cmake version 3.28.1
[OK] MSVC Compiler found
[OK] NSFW Model
[OK] BGE Semantic Model
```

## Project Structure

```
FlyKylin/
├── tools/developer/       # Developer scripts
│   ├── start-vsdevcmd.ps1         # Launch VS Dev Prompt ⭐
│   ├── configure-environment.ps1  # Configure paths
│   ├── verify-environment.ps1     # Check dependencies
│   ├── install-onnx-runtime.ps1   # Install ONNX
│   └── install-protobuf.ps1       # Install Protobuf
├── docs/
│   ├── requirements/      # Core requirements
│   └── 飞秋方案.md         # Technical design
├── CMakeLists.txt         # Build configuration
└── README.md              # Project README
```

## Next Steps

Once the build succeeds:

1. Review requirements: `docs/requirements/FlyKylin需求.md`
2. Check Sprint 0 plan: `docs/requirements/Sprint_0_环境验证.md`
3. Read status: `SPRINT0_STATUS.md`

## Need Help?

- Tool documentation: `tools/developer/README.md`
- Environment guide: `docs/环境安装指南.md`
- Sprint status: `SPRINT0_STATUS.md`

---

**Key Principle**: No encryption, no authentication - Focus on P2P + AI features
