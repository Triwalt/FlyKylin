# Quick Start: GitHub Actions CI/CD

## 🚀 一分钟了解自动构建

提交代码到 GitHub 后，GitHub Actions 会自动：
1. ✅ 构建 4 个平台版本
2. ✅ 运行所有测试
3. ✅ 打包发布产物
4. ✅ 创建 GitHub Release（如果是 tag）

**无需手动操作，全程自动化！**

---

## 📋 前置要求

### 在 GitHub 上

1. **创建 GitHub 仓库**
   ```bash
   # 如果还没有创建仓库
   gh repo create FlyKylin --public
   ```

2. **推送代码**
   ```bash
   git remote add origin https://github.com/YOUR_USERNAME/FlyKylin.git
   git push -u origin main
   ```

3. **启用 Actions**
   - 进入仓库 Settings → Actions → General
   - 选择 "Allow all actions and reusable workflows"

---

## 🔄 开发流程

### 日常开发

```bash
# 1. 开发功能
git checkout -b feature/my-feature
# ... 编写代码 ...

# 2. 本地测试（可选）
.\tools\developer\test-ci-build.ps1 -Platform windows

# 3. 提交并推送
git add .
git commit -m "feat: add new feature"
git push origin feature/my-feature

# 4. 创建 Pull Request
gh pr create --title "Add new feature" --body "Description"
```

**自动触发**: PR 创建后，GitHub Actions 会自动构建并运行测试

**查看结果**: PR 页面会显示构建状态 ✅ 或 ❌

---

### 发布版本

```bash
# 1. 确保 main 分支是最新的
git checkout main
git pull origin main

# 2. 创建并推送 tag
git tag -a v0.1.0 -m "Release v0.1.0 - Sprint 1 Alpha

Main features:
- UDP peer discovery
- Basic UI with peer list
- Cross-platform support (Windows/Linux)"

git push origin v0.1.0
```

**自动执行**:
1. ✅ 构建所有平台（Windows AMD64/ARM64, Linux AMD64/ARM64）
2. ✅ 运行测试套件
3. ✅ 打包发布文件
4. ✅ 生成 SHA256 校验和
5. ✅ 创建 GitHub Release
6. ✅ 上传所有发布产物

**5-10 分钟后**，访问: `https://github.com/YOUR_USERNAME/FlyKylin/releases`

---

## 📦 下载发布版本

### 用户下载

发布后，用户可以直接下载：

```
https://github.com/YOUR_USERNAME/FlyKylin/releases/latest
```

**文件列表**:
- `FlyKylin-v0.1.0-windows-amd64.zip` (Windows 64位)
- `FlyKylin-v0.1.0-windows-arm64.zip` (Windows ARM)
- `FlyKylin-v0.1.0-linux-amd64.tar.gz` (Linux 64位)
- `FlyKylin-v0.1.0-linux-arm64.tar.gz` (Linux ARM/RK3566)
- `*.sha256` (校验和文件)

### 验证下载

```powershell
# Windows
$expectedHash = Get-Content FlyKylin-v0.1.0-windows-amd64.zip.sha256
$actualHash = (Get-FileHash FlyKylin-v0.1.0-windows-amd64.zip -Algorithm SHA256).Hash
if ($expectedHash -eq $actualHash) {
    Write-Host "✓ File integrity verified"
} else {
    Write-Host "✗ File corrupted!"
}
```

```bash
# Linux
sha256sum -c FlyKylin-v0.1.0-linux-amd64.tar.gz.sha256
```

---

## 🔍 查看构建状态

### 方式1: GitHub Web UI

1. 进入仓库主页
2. 点击 **Actions** 标签
3. 查看最近的 workflow 运行

### 方式2: GitHub CLI

```bash
# 查看最近的运行
gh run list --workflow=build-and-release.yml

# 查看特定运行的详情
gh run view <RUN_ID>

# 查看日志
gh run view <RUN_ID> --log
```

### 方式3: Badge（添加到 README）

在 `README.md` 中添加：

```markdown
[![Build Status](https://github.com/YOUR_USERNAME/FlyKylin/actions/workflows/build-and-release.yml/badge.svg)](https://github.com/YOUR_USERNAME/FlyKylin/actions)
```

显示效果: ![Build Status](https://img.shields.io/badge/build-passing-brightgreen)

---

## 🛠️ 常见问题

### Q: 构建失败怎么办？

**A**: 查看构建日志

```bash
# 获取失败的运行 ID
gh run list --workflow=build-and-release.yml --status=failure

# 查看日志
gh run view <FAILED_RUN_ID> --log

# 或在网页上查看
# https://github.com/YOUR_USERNAME/FlyKylin/actions
```

**常见错误**:
- ❌ Qt 安装失败 → 检查 Qt 版本是否可用
- ❌ 测试失败 → 本地运行测试，修复后再推送
- ❌ vcpkg 超时 → 重新运行 workflow（偶发）

### Q: 如何重新运行构建？

**A**: 

1. **通过 Web UI**: Actions 页面 → 选择运行 → "Re-run jobs"
2. **通过 CLI**:
   ```bash
   gh run rerun <RUN_ID>
   ```

### Q: 构建太慢怎么办？

**A**: 当前构建时间（首次）:
- Windows: ~20 分钟
- Linux: ~15 分钟
- ARM64: ~35 分钟（QEMU 模拟）

**优化方法**:
1. ✅ 已启用 vcpkg 缓存（节省 ~5-10 分钟）
2. ✅ 已启用 Qt 缓存（节省 ~3-5 分钟）
3. 🔜 可添加 ccache（节省 ~5 分钟）
4. 🔜 使用自托管 ARM64 runner（快 10 倍）

### Q: 如何禁用某个平台的构建？

**A**: 编辑 `.github/workflows/build-and-release.yml`

```yaml
# 注释掉不需要的 job
jobs:
  # build-windows-arm64:  # 暂时禁用 ARM64
  #   runs-on: windows-latest
  #   ...
```

### Q: 如何只测试 PR 而不发布？

**A**: Workflow 已自动配置：
- ✅ PR 触发 → 只构建和测试，不创建 Release
- ✅ Tag 触发 → 构建、测试、打包、发布

---

## 📊 成本估算

### GitHub Actions 免费额度

**公开仓库**: 
- ✅ **完全免费，无限制**

**私有仓库**:
- 免费额度: 2000 分钟/月
- FlyKylin 单次构建: ~80 分钟（4 平台并行）
- 可构建次数: ~25 次/月

**超出后**:
- Windows/Linux: $0.008/分钟
- 月成本估算: ~$10-20（频繁发布）

**建议**: 
- 开发期间使用公开仓库（免费）
- 发布后可转为私有

---

## 🔐 安全性

### Secrets 管理

暂时不需要 secrets，未来可能需要：

```yaml
# 示例：代码签名
- name: Sign binary
  env:
    SIGNING_CERT: ${{ secrets.WINDOWS_SIGNING_CERT }}
  run: |
    # 签名命令
```

**添加 Secret**:
1. Settings → Secrets and variables → Actions
2. New repository secret
3. 输入名称和值

### 依赖安全

所有依赖来自可信源：
- ✅ Qt: 官方网站
- ✅ vcpkg: Microsoft 官方
- ✅ Actions: GitHub 验证

---

## 📈 监控和通知

### Slack 通知（可选）

```yaml
# 添加到 workflow
- name: Notify Slack
  if: failure()
  uses: 8398a7/action-slack@v3
  with:
    status: ${{ job.status }}
    webhook_url: ${{ secrets.SLACK_WEBHOOK }}
```

### Email 通知

GitHub 默认会发送邮件通知：
- ✅ 构建失败时
- ✅ PR 合并后

**配置**: Settings → Notifications

---

## 🎯 最佳实践

### 1. 提交前本地测试

```bash
# Windows
.\tools\developer\test-ci-build.ps1

# 确保测试通过后再 push
```

### 2. 使用语义化版本

```
v0.1.0 - Sprint 1 Alpha
v0.2.0 - Sprint 2 Beta
v1.0.0 - First stable release
```

### 3. 编写清晰的 Release Notes

```bash
git tag -a v0.1.0 -m "Release v0.1.0

Features:
- UDP peer discovery
- Real-time peer list
- Cross-platform support

Breaking Changes:
- None (first release)

Known Issues:
- GoogleTest Debug mode not available
"
```

### 4. 定期清理旧 Releases

保留：
- 最新 3 个版本
- 重要里程碑版本（v1.0.0, v2.0.0）

删除：
- 过时的 alpha/beta 版本

---

## 📚 相关文档

- **详细 CI/CD 文档**: [docs/ci-cd.md](./ci-cd.md)
- **工作流文件**: [.github/workflows/build-and-release.yml](../.github/workflows/build-and-release.yml)
- **构建脚本**: [tools/developer/](../tools/developer/)

---

## ✅ 快速检查清单

发布前确认：

- [ ] 代码已提交到 main 分支
- [ ] 本地测试通过
- [ ] 更新了版本号（如需要）
- [ ] 准备了 Release Notes
- [ ] 创建并推送了 tag
- [ ] 等待 5-10 分钟构建完成
- [ ] 验证 GitHub Release 页面
- [ ] 下载并测试发布包
- [ ] 通知用户新版本

---

**需要帮助?** 
- 查看 [详细文档](./ci-cd.md)
- 提交 [Issue](https://github.com/YOUR_USERNAME/FlyKylin/issues)
- 查看 [GitHub Actions 文档](https://docs.github.com/en/actions)

**Last Updated**: 2024-11-18
