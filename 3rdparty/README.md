# Third-Party Dependencies Management

This directory contains third-party libraries and tools for FlyKylin project.

## Directory Structure

```
3rdparty/
├── onnxruntime/         # ONNX Runtime (Windows DirectML, RK3566 NPU)
├── rknn-toolkit2/       # Rockchip NPU Toolkit (RK3566 platform)
├── rknn_api/            # RKNN Runtime API
└── protobuf/            # Protocol Buffers
```

## Important Notes

⚠️ **These directories are excluded from Git repository via `.gitignore`**

All third-party libraries should be installed through:
1. **Package managers** (vcpkg, apt, etc.)
2. **Official download links**
3. **Platform-specific installers**

## RKNN Toolkit 2 (RK3566 Platform)

### Overview

RKNN Toolkit 2 is Rockchip's neural network inference SDK for NPU acceleration on RK3566/RK3568 chips.

### Download

**Official Source**: [RKNN-Toolkit2 GitHub](https://github.com/airockchip/rknn-toolkit2)

**Version**: 2.3.2 (or latest stable)

### Installation

#### For Development (Windows/Linux x64)

```bash
# 1. Clone the repository (do not commit to project)
cd E:/Project/FlyKylin/3rdparty
git clone https://github.com/airockchip/rknn-toolkit2.git

# 2. Install Python package (optional, for model conversion)
pip install rknn-toolkit2
```

#### For Deployment (RK3566 Linux ARM64)

```bash
# On RK3566 device
# Install runtime library
sudo dpkg -i rknpu2-rk356x-linux_x.x.x.deb

# Start RKNN server
sudo systemctl start rknn_server
```

### File Size Warning

⚠️ **Do NOT commit RKNN Toolkit to Git!**

- Total size: ~2GB+ (includes models, docs, examples)
- Contains large binary files (.rknn models, .so libraries)
- Already excluded in `.gitignore`

### What to Keep in Project

**Only commit**:
- Model files in `model/rknn/` (if small, <10MB)
- Integration code in `src/platform/rk3566/`
- Documentation references

**Do NOT commit**:
- `3rdparty/rknn-toolkit2/` (entire toolkit)
- Large .rknn model files (>10MB)
- Example code from toolkit

## ONNX Runtime

### Download

**Official**: [ONNX Runtime Releases](https://github.com/microsoft/onnxruntime/releases)

**Windows**: 
```
onnxruntime-win-x64-gpu-X.X.X.zip (DirectML)
```

**Linux ARM64**:
```
onnxruntime-linux-aarch64-X.X.X.tgz
```

### Installation

```bash
# Extract to 3rdparty/
cd 3rdparty/
unzip onnxruntime-win-x64-gpu-X.X.X.zip
mv onnxruntime-* onnxruntime/
```

## Protocol Buffers

### Installation via vcpkg

```bash
vcpkg install protobuf:x64-windows
vcpkg install protobuf:arm64-linux
```

Installed location: `vcpkg/installed/`

## Developer Setup Script

Run the setup script to install all dependencies:

```bash
# Windows
.\tools\developer\setup\install-dependencies.cmd

# Or manual install
vcpkg install onnxruntime:x64-windows
# Download RKNN manually from GitHub
```

## CI/CD Notes

For automated builds, dependencies should be:
1. **Cached** in CI environment
2. **Downloaded** from official sources
3. **Not stored** in Git repository

Example GitHub Actions cache:

```yaml
- name: Cache dependencies
  uses: actions/cache@v3
  with:
    path: |
      3rdparty/onnxruntime
      3rdparty/rknn-toolkit2
    key: ${{ runner.os }}-deps-${{ hashFiles('**/dependency-versions.txt') }}
```

## Cleanup

If you need to clean up third-party files:

```bash
# Windows PowerShell
Remove-Item -Recurse -Force 3rdparty/onnxruntime
Remove-Item -Recurse -Force 3rdparty/rknn-toolkit2
Remove-Item -Recurse -Force 3rdparty/rknn_api

# Then re-install
.\tools\developer\setup\install-dependencies.cmd
```

## Version Management

Track dependency versions in:
```
docs/dependencies.md
```

Current versions:
- ONNX Runtime: 1.16.0+
- RKNN Toolkit 2: 2.3.2
- Protobuf: 3.21.0+

## Support

- **RKNN Issues**: [GitHub Issues](https://github.com/airockchip/rknn-toolkit2/issues)
- **ONNX Runtime**: [GitHub Issues](https://github.com/microsoft/onnxruntime/issues)
- **Project-specific**: See `docs/troubleshooting.md`

---

**Last Updated**: 2024-11-18  
**Maintainer**: FlyKylin Development Team
