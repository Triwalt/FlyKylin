# CI/CD Documentation

## GitHub Actions Build Pipeline

FlyKylin uses GitHub Actions for automated cross-platform builds and releases.

### Supported Platforms

| Platform | Architecture | Build Status | Download |
|----------|-------------|--------------|----------|
| Windows | AMD64 (x64) | ✅ Supported | `.zip` |
| Windows | ARM64 | ✅ Supported | `.zip` |
| Linux | AMD64 (x64) | ✅ Supported | `.tar.gz` |
| Linux | ARM64 (RK3566) | ✅ Supported | `.tar.gz` |

---

## Workflow Overview

### Workflow File

`.github/workflows/build-and-release.yml`

### Trigger Conditions

**Automatic triggers**:
- Push to `main` or `develop` branch
- Pull requests to `main`
- Git tags starting with `v*` (e.g., `v0.1.0`)

**Manual trigger**:
```bash
# Via GitHub UI: Actions tab -> Build and Release -> Run workflow
# Or using GitHub CLI:
gh workflow run build-and-release.yml
```

---

## Build Process

### 1. Windows AMD64 Build

**Runner**: `windows-latest` (Windows Server 2022)

**Steps**:
1. Checkout code
2. Setup MSVC (Visual Studio 2022)
3. Install Qt 6.5.3 (win64_msvc2019_64)
4. Setup vcpkg
5. Install Protobuf via vcpkg
6. Configure CMake with MSVC generator
7. Build (Release mode, parallel build)
8. Run tests (CTest)
9. Package with windeployqt
10. Create ZIP archive + SHA256 checksum
11. Upload artifacts

**Build time**: ~15-20 minutes

---

### 2. Windows ARM64 Build

**Runner**: `windows-latest`

**Differences from AMD64**:
- Uses `win64_msvc2019_arm64` Qt package
- CMake generator: `Visual Studio 17 2022 -A ARM64`
- Tests are skipped (cannot run ARM64 binaries on x64 host)

**Build time**: ~15-20 minutes

---

### 3. Linux AMD64 Build

**Runner**: `ubuntu-22.04`

**Steps**:
1. Checkout code
2. Install system dependencies (Mesa, XCB, etc.)
3. Install Qt 6.5.3 (gcc_64)
4. Setup vcpkg
5. Install Protobuf via vcpkg
6. Configure CMake with Ninja generator
7. Build (Release mode, parallel build)
8. Run tests (CTest)
9. Package with Qt libraries bundled
10. Create launcher script
11. Create TAR.GZ archive + SHA256 checksum
12. Upload artifacts

**Build time**: ~10-15 minutes

---

### 4. Linux ARM64 Build

**Runner**: `ubuntu-22.04` with QEMU

**Special setup**:
- Uses Docker with QEMU for ARM64 emulation
- Builds in `ubuntu:22.04` ARM64 container
- Uses system Qt or downloads pre-built Qt

**Build time**: ~30-40 minutes (slower due to emulation)

---

## Release Process

### Creating a Release

1. **Ensure all changes are committed and pushed**

2. **Create and push a tag**:
   ```bash
   git tag -a v0.1.0 -m "Release v0.1.0 - Sprint 1"
   git push origin v0.1.0
   ```

3. **GitHub Actions automatically**:
   - Builds all 4 platforms
   - Runs tests (where applicable)
   - Creates release artifacts
   - Generates release notes
   - Creates GitHub Release
   - Uploads all packages

4. **Verify the release**:
   - Go to GitHub Releases page
   - Check all 4 platform packages are present
   - Verify SHA256 checksums
   - Test download and installation

### Release Artifacts

For tag `v0.1.0`, the following files are created:

```
FlyKylin-v0.1.0-windows-amd64.zip
FlyKylin-v0.1.0-windows-amd64.zip.sha256
FlyKylin-v0.1.0-windows-arm64.zip
FlyKylin-v0.1.0-windows-arm64.zip.sha256
FlyKylin-v0.1.0-linux-amd64.tar.gz
FlyKylin-v0.1.0-linux-amd64.tar.gz.sha256
FlyKylin-v0.1.0-linux-arm64.tar.gz
FlyKylin-v0.1.0-linux-arm64.tar.gz.sha256
```

---

## Dependencies Management

### vcpkg Integration

The workflow uses vcpkg for C++ dependencies:

**Installed packages**:
- `protobuf` (all platforms)

**Cache strategy**:
- vcpkg binary cache is enabled
- Speeds up subsequent builds
- Cache key based on vcpkg commit hash

### Qt Installation

Uses `jurplel/install-qt-action@v3`:

**Advantages**:
- Automatic download and caching
- Supports all platforms
- Includes `windeployqt` / `linuxdeployqt`

**Version**: Qt 6.5.3 (configurable via `env.QT_VERSION`)

---

## Testing

### Automated Tests

**Platforms with tests**:
- ✅ Windows AMD64
- ✅ Linux AMD64

**Skipped tests**:
- ⏭️ Windows ARM64 (cannot run on x64 host)
- ⏭️ Linux ARM64 (slow emulation, optional)

**Test command**:
```bash
ctest -C Release --output-on-failure
```

**Test coverage**:
- Unit tests: PeerNode, PeerDiscovery
- Integration tests: (to be added)

---

## Troubleshooting

### Build Failures

#### Qt installation fails

**Symptom**: `install-qt-action` fails to download Qt

**Solution**:
- Check Qt version is available
- Try using a different Qt mirror
- Update `jurplel/install-qt-action` to latest version

#### vcpkg build fails

**Symptom**: Protobuf or other package fails to build

**Solution**:
```yaml
# Update vcpkg commit in workflow
env:
  VCPKG_COMMIT: '2024.03.25'  # Use a stable tag
```

#### Tests fail

**Symptom**: CTest reports test failures

**Steps**:
1. Check test logs in Actions output
2. Reproduce locally:
   ```bash
   cmake --build build --config Release
   cd build && ctest -C Release -V
   ```
3. Fix failing tests
4. Push fixes

#### ARM64 build times out

**Symptom**: Linux ARM64 build exceeds 60 minutes

**Solutions**:
- Reduce parallelism: `-j2` instead of `-j$(nproc)`
- Use pre-built Qt instead of building from source
- Consider using native ARM64 runner (GitHub-hosted or self-hosted)

---

## Local Testing

### Test builds locally before pushing

#### Windows (PowerShell):
```powershell
# Install dependencies
vcpkg install protobuf:x64-windows

# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="./vcpkg/scripts/buildsystems/vcpkg.cmake"

# Build
cmake --build build --config Release -j4

# Test
cd build
ctest -C Release --output-on-failure
```

#### Linux (Bash):
```bash
# Install dependencies
sudo apt-get install build-essential cmake ninja-build qt6-base-dev
vcpkg install protobuf:x64-linux

# Configure
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build -j$(nproc)

# Test
cd build && ctest --output-on-failure
```

---

## Performance Optimization

### Build Time Optimization

**Current times**:
- Windows AMD64: ~15-20 min
- Windows ARM64: ~15-20 min  
- Linux AMD64: ~10-15 min
- Linux ARM64: ~30-40 min
- **Total parallel**: ~30-40 min

**Optimization strategies**:

1. **Cache vcpkg binaries**:
   ```yaml
   - uses: actions/cache@v3
     with:
       path: ~/.cache/vcpkg
       key: ${{ runner.os }}-vcpkg-${{ hashFiles('vcpkg.json') }}
   ```

2. **Use ccache**:
   ```yaml
   - uses: hendrikmuhs/ccache-action@v1
   ```

3. **Reduce test coverage** (for pre-release builds):
   ```yaml
   - name: Run tests
     if: github.event_name == 'push' && startsWith(github.ref, 'refs/tags/')
   ```

4. **Self-hosted runners** for ARM64:
   - Use native ARM64 hardware (e.g., RK3588 board)
   - 5-10x faster than QEMU emulation

---

## Security Considerations

### Secrets Management

No secrets are currently required.

**Future considerations**:
- Code signing certificates (Windows)
- GPG signing keys (Linux packages)
- Deployment tokens (for auto-update)

**Best practices**:
```yaml
- name: Sign package
  if: github.event_name == 'push' && startsWith(github.ref, 'refs/tags/')
  env:
    SIGNING_KEY: ${{ secrets.CODE_SIGNING_KEY }}
  run: |
    # Sign binaries
```

### Dependency Security

**Supply chain**:
- vcpkg packages: Official Microsoft registry
- Qt: Official Qt Company releases
- All dependencies pinned to specific versions

**Vulnerability scanning**:
```yaml
- name: Run security scan
  uses: aquasecurity/trivy-action@master
  with:
    scan-type: 'fs'
    scan-ref: '.'
```

---

## Monitoring and Notifications

### Build Status Badge

Add to README.md:

```markdown
[![Build Status](https://github.com/YOUR_ORG/FlyKylin/actions/workflows/build-and-release.yml/badge.svg)](https://github.com/YOUR_ORG/FlyKylin/actions)
```

### Slack/Discord Notifications

```yaml
- name: Notify on failure
  if: failure()
  uses: 8398a7/action-slack@v3
  with:
    status: ${{ job.status }}
    text: 'Build failed for ${{ matrix.platform }}'
    webhook_url: ${{ secrets.SLACK_WEBHOOK }}
```

---

## Advanced Configuration

### Matrix Strategy

For more flexible builds:

```yaml
strategy:
  matrix:
    include:
      - os: windows-latest
        arch: x64
        qt_arch: win64_msvc2019_64
      - os: windows-latest
        arch: ARM64
        qt_arch: win64_msvc2019_arm64
      - os: ubuntu-22.04
        arch: x64
        qt_arch: gcc_64
      - os: ubuntu-22.04
        arch: ARM64
        qt_arch: linux_arm64
```

### Conditional Builds

Skip builds for documentation changes:

```yaml
on:
  push:
    paths-ignore:
      - 'docs/**'
      - '**.md'
```

---

## Future Improvements

### Planned enhancements:

1. **Code coverage reports**
   - Upload to Codecov
   - Generate HTML reports

2. **Performance benchmarks**
   - Track build time trends
   - Measure binary size

3. **Auto-update mechanism**
   - Check for updates in app
   - Download and install new versions

4. **macOS support**
   - Add macOS AMD64 build
   - Add macOS ARM64 (Apple Silicon) build

5. **AppImage / Flatpak for Linux**
   - More portable Linux packages

6. **Chocolatey / Winget for Windows**
   - Easy installation via package managers

---

## References

- **GitHub Actions Docs**: https://docs.github.com/en/actions
- **Qt Installer Action**: https://github.com/jurplel/install-qt-action
- **vcpkg Action**: https://github.com/lukka/run-vcpkg
- **CMake Docs**: https://cmake.org/documentation/

---

**Last Updated**: 2024-11-18  
**Maintainer**: FlyKylin DevOps Team
