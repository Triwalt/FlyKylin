---
description: 阶段性工作整理、构建与分发工作流
---

# 阶段性交付工作流

## 使用场景
在Sprint结束或重要里程碑完成时，进行阶段性工作整理、构建产物打包和版本发布。

## 前置条件
- [ ] Sprint/里程碑目标已完成
- [ ] 所有测试通过
- [ ] Code Review已完成
- [ ] 文档已更新
- [ ] 质量指标达标

---

## 工作流步骤

### 第1步：阶段性工作整理

#### 1.1 Sprint回顾

```
@cascade 生成Sprint回顾报告：

参考当前Sprint文档：
@docs/sprints/active/sprint_X/

生成内容：
1. **Sprint目标达成情况**
   - 计划的Story: X个
   - 完成的Story: X个
   - 完成率: X%
   - 未完成原因分析

2. **交付成果清单**
   - 已实现功能列表
   - 新增代码行数
   - 测试用例数量
   - 文档更新情况

3. **质量指标汇总**
   - 测试覆盖率: X%
   - Bug数量: X个
   - 代码审查问题: X个
   - 性能指标达成情况

4. **团队协作反思**
   - 做得好的地方（继续保持）
   - 遇到的问题（需要改进）
   - 改进行动计划

输出文件：docs/sprints/active/sprint_X/retrospective.md
```

#### 1.2 技术债务清单

```
@cascade 整理技术债务：

分析以下内容：
- Code Review中标记的Minor/Suggestion问题
- TODO/FIXME注释
- 临时解决方案
- 性能优化点

输出格式：
| 债务ID | 描述 | 影响范围 | 优先级 | 预计工作量 | 计划解决Sprint |
|--------|------|----------|--------|------------|----------------|
| TD-001 | ... | ... | High | 4h | Sprint 2 |

输出文件：docs/technical-debt.md
```

#### 1.3 成果演示材料

```
@cascade 准备演示材料：

基于：
@docs/sprints/active/sprint_X/DEMO.md

生成：
1. **演示PPT大纲**
   - 封面：Sprint X 成果演示
   - 目标与范围
   - 功能演示截图/录屏
   - 技术亮点
   - 数据指标
   - 下一步计划

2. **演示脚本**
   - 开场白
   - 功能演示步骤
   - 关键亮点说明
   - Q&A准备

3. **用户手册更新**
   - 新功能使用说明
   - 常见问题FAQ
   - 故障排查指南

输出文件：docs/sprints/active/sprint_X/presentation/
```

---

### 第2步：版本规划与发布准备

#### 2.1 版本号确定

```bash
# 语义化版本规则
# MAJOR.MINOR.PATCH
# - MAJOR: 不兼容的API变更
# - MINOR: 向后兼容的功能新增
# - PATCH: 向后兼容的问题修复

# 示例：
# Sprint 1完成 → v0.1.0 (首个Alpha版本)
# Sprint 2完成 → v0.2.0 (新增功能)
# Bug修复 → v0.2.1 (补丁版本)

# 确定版本号
CURRENT_VERSION="v0.1.0"  # 根据实际情况修改
```

#### 2.2 更新版本信息

```
@cascade 更新项目版本信息：

需要修改的文件：
1. CMakeLists.txt
   - project(FlyKylin VERSION 0.1.0)

2. src/version.h
   ```cpp
   #define FLYKYLIN_VERSION_MAJOR 0
   #define FLYKYLIN_VERSION_MINOR 1
   #define FLYKYLIN_VERSION_PATCH 0
   #define FLYKYLIN_VERSION_STRING "0.1.0"
   #define FLYKYLIN_BUILD_DATE __DATE__
   #define FLYKYLIN_BUILD_TIME __TIME__
   ```

3. README.md
   - 版本徽章
   - 版本历史

4. docs/changelog.md
   - 新版本发布说明
```

#### 2.3 生成Release Notes

```
@cascade 生成Release Notes：

格式：
# FlyKylin v0.1.0 Release Notes

**发布日期**: 2024-11-18

## 🎯 本版本亮点
- [简要描述最重要的3-5个功能或改进]

## ✨ 新功能
- [US-001] UDP节点自动发现
  * 局域网内P2P自动发现
  * 心跳机制保持在线状态
  * 超时自动离线检测

- [US-003] 在线用户列表UI
  * 实时显示在线用户
  * 搜索过滤功能
  * MVVM架构设计

## 🐛 Bug修复
- 修复PeerNode测试失败问题
- 修复GoogleTest宏混用问题

## ⚡ 性能优化
- UDP节点发现延迟 < 5秒
- UI响应时间 < 100ms

## 🔧 技术改进
- 完善错误处理机制
- 提升测试覆盖率至100%（核心逻辑）
- 代码规范优化

## 📊 质量指标
- 测试通过率: 100% (16/16)
- 代码覆盖率: 100% (核心逻辑)
- 编译警告: 0
- 代码审查评分: 9.2/10

## 🚀 系统要求
- Windows 10/11 x64
- Qt 6.9.3+
- Visual Studio 2022
- 最低4GB内存

## 📦 下载地址
- Windows版本: [ai-feiqiu-v0.1.0-windows-x64.exe]
- 用户手册: [docs/user-guide.md]
- 开发文档: [docs/developer-guide.md]

## ⚠️ 已知问题
- GoogleTest Debug模式不可用（非阻塞）
- 当前使用简单文本协议（Sprint 2将改为Protobuf）

## 🔜 下一版本计划 (v0.2.0)
- [US-002] TCP长连接实现
- [US-004] 文本消息收发
- Protobuf协议集成

---

**完整变更**: [v0.0.0...v0.1.0](链接到Git对比)
**反馈**: [提交Issue](链接)

输出文件：releases/v0.1.0/RELEASE_NOTES.md
```

---

### 第3步：构建与打包

#### 3.1 清理与准备

// turbo
```powershell
# 清理之前的构建
cd e:\Project\FlyKylin
if (Test-Path build) { Remove-Item -Recurse -Force build }
if (Test-Path releases\temp) { Remove-Item -Recurse -Force releases\temp }

# 创建发布目录
New-Item -ItemType Directory -Force -Path releases\v0.1.0
New-Item -ItemType Directory -Force -Path releases\temp
```

#### 3.2 Release构建

// turbo
```powershell
# 执行Release构建
.\build-release.cmd

# 验证构建结果
if ($LASTEXITCODE -ne 0) {
    Write-Error "构建失败，请检查错误日志"
    exit 1
}

Write-Host "✓ 构建成功" -ForegroundColor Green
```

#### 3.3 运行完整测试

// turbo
```powershell
# 运行所有测试
.\run-tests.cmd

# 验证测试结果
if ($LASTEXITCODE -ne 0) {
    Write-Error "测试失败，请修复后再发布"
    exit 1
}

Write-Host "✓ 所有测试通过" -ForegroundColor Green
```

#### 3.4 质量检查

```powershell
# 代码质量检查
Write-Host "执行质量检查..." -ForegroundColor Yellow

# 检查编译警告
Write-Host "检查编译警告..." -ForegroundColor Cyan
# 应该为0个警告

# 检查测试覆盖率
Write-Host "检查测试覆盖率..." -ForegroundColor Cyan
# 核心逻辑应该≥80%

# 生成质量报告
$QualityReport = @"
# 质量检查报告 - v0.1.0

**检查时间**: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## 编译检查
- ✅ 编译通过
- ✅ 零警告

## 测试检查
- ✅ 单元测试: 16/16 通过
- ✅ 测试覆盖率: 100% (核心逻辑)

## 代码质量
- ✅ 代码规范: 符合标准
- ✅ Code Review: 9.2/10

## 性能检查
- ✅ 节点发现: < 5秒
- ✅ UI响应: < 100ms
- ✅ 内存占用: ~50MB

## 结论
✅ **通过质量检查，可以发布**
"@

$QualityReport | Out-File -FilePath "releases\v0.1.0\QUALITY_REPORT.md" -Encoding UTF8
Write-Host "✓ 质量检查通过" -ForegroundColor Green
```

#### 3.5 打包发布产物

```powershell
# 创建发布包
Write-Host "打包发布产物..." -ForegroundColor Yellow

$VERSION = "0.1.0"
$PACKAGE_NAME = "FlyKylin-v$VERSION-windows-x64"
$PACKAGE_DIR = "releases\temp\$PACKAGE_NAME"

# 创建打包目录
New-Item -ItemType Directory -Force -Path $PACKAGE_DIR

# 复制可执行文件
Copy-Item "build\bin\Release\FlyKylin.exe" -Destination "$PACKAGE_DIR\"

# 复制Qt依赖（使用windeployqt）
& "D:\Qt\6.9.3\msvc2022_64\bin\windeployqt.exe" `
    --release `
    --no-translations `
    --no-system-d3d-compiler `
    "$PACKAGE_DIR\FlyKylin.exe"

# 复制文档
Copy-Item "README.md" -Destination "$PACKAGE_DIR\"
Copy-Item "releases\v$VERSION\RELEASE_NOTES.md" -Destination "$PACKAGE_DIR\"
Copy-Item "docs\user-guide.md" -Destination "$PACKAGE_DIR\docs\" -Recurse -Force

# 复制许可证
Copy-Item "LICENSE" -Destination "$PACKAGE_DIR\" -ErrorAction SilentlyContinue

# 创建压缩包
Compress-Archive -Path "$PACKAGE_DIR\*" `
    -DestinationPath "releases\v$VERSION\$PACKAGE_NAME.zip" `
    -Force

# 计算文件哈希
$hash = Get-FileHash "releases\v$VERSION\$PACKAGE_NAME.zip" -Algorithm SHA256
$hash.Hash | Out-File -FilePath "releases\v$VERSION\$PACKAGE_NAME.zip.sha256" -Encoding ASCII

Write-Host "✓ 打包完成: releases\v$VERSION\$PACKAGE_NAME.zip" -ForegroundColor Green
Write-Host "  文件大小: $((Get-Item "releases\v$VERSION\$PACKAGE_NAME.zip").Length / 1MB) MB" -ForegroundColor Gray
Write-Host "  SHA256: $($hash.Hash)" -ForegroundColor Gray
```

---

### 第4步：版本标记与发布

#### 4.1 Git提交与标签

```bash
# 提交版本更新
git add .
git commit -m "chore(release): prepare v0.1.0 release

- Update version to 0.1.0
- Add release notes
- Update documentation
- Generate quality report

Sprint 1 Deliverables:
- [US-001] UDP Node Discovery (100%)
- [US-003] Basic UI (100%)
- Test Coverage: 100% core logic
- All tests passing: 16/16

Quality Score: 9.2/10"

# 创建Git标签
git tag -a v0.1.0 -m "Release v0.1.0 - Sprint 1 完成

主要功能：
- UDP节点自动发现
- 在线用户列表UI
- MVVM架构实现

质量指标：
- 测试通过率: 100%
- 代码覆盖率: 100% (核心)
- Code Review: 9.2/10"

# 推送到远程
git push origin main
git push origin v0.1.0
```

#### 4.2 创建GitHub Release

```
@cascade 生成GitHub Release内容：

基于：
@releases/v0.1.0/RELEASE_NOTES.md

GitHub Release格式：
---
Tag: v0.1.0
Target: main
Title: FlyKylin v0.1.0 - Sprint 1 Alpha Release

Description:
[粘贴RELEASE_NOTES内容]

Assets:
- FlyKylin-v0.1.0-windows-x64.zip
- FlyKylin-v0.1.0-windows-x64.zip.sha256
- Source code (zip)
- Source code (tar.gz)

Pre-release: ☑ (标记为预发布版本)
---
```

---

### 第5步：归档与清理

#### 5.1 Sprint归档

```powershell
# 归档当前Sprint
$SPRINT_NUM = 1
$ARCHIVE_DATE = Get-Date -Format "yyyy-MM-dd"

# 移动到归档目录
Move-Item "docs\sprints\active\sprint_$SPRINT_NUM" `
    -Destination "docs\sprints\archive\sprint_${SPRINT_NUM}_${ARCHIVE_DATE}" `
    -Force

Write-Host "✓ Sprint $SPRINT_NUM 已归档" -ForegroundColor Green
```

#### 5.2 更新项目状态

```
@cascade 更新项目主文档：

1. README.md
   - 更新版本徽章
   - 更新功能列表
   - 更新下载链接

2. docs/roadmap.md
   - 标记Sprint 1为完成
   - 更新Sprint 2计划
   - 调整里程碑时间

3. docs/changelog.md
   - 添加v0.1.0条目

4. SPRINT1_PROGRESS.md
   - 标记为已完成
   - 移动到归档
```

#### 5.3 清理临时文件

// turbo
```powershell
# 清理临时文件
Remove-Item -Recurse -Force "releases\temp"
Remove-Item -Recurse -Force "build\CMakeFiles" -ErrorAction SilentlyContinue

Write-Host "✓ 清理完成" -ForegroundColor Green
```

---

### 第6步：发布通知与分发

#### 6.1 生成发布公告

```
@cascade 生成发布公告：

面向：团队成员、测试人员、早期用户

内容：
---
主题：FlyKylin v0.1.0 发布 - Sprint 1 Alpha版本

各位，

很高兴宣布 FlyKylin v0.1.0 正式发布！这是我们的第一个Alpha版本。

🎉 主要功能：
• UDP节点自动发现 - 局域网内P2P自动发现
• 在线用户列表 - 实时显示和搜索
• 现代化UI - 基于Qt 6的美观界面

📊 质量指标：
• 测试通过率: 100% (16/16)
• 代码覆盖率: 100% (核心逻辑)
• Code Review评分: 9.2/10

📦 下载地址：
https://github.com/xxx/releases/tag/v0.1.0

📖 发布说明：
详见 RELEASE_NOTES.md

🔜 下一步：
Sprint 2将实现TCP长连接和消息收发功能。

欢迎测试并反馈问题！

---
FlyKylin开发团队
2024-11-18
```

#### 6.2 更新分发渠道

```markdown
# 分发渠道清单

- [ ] GitHub Releases (自动)
- [ ] 内部文件服务器
- [ ] 测试人员邮件列表
- [ ] 项目Wiki更新
- [ ] 团队协作工具通知（如钉钉、Slack）
```

---

## 📋 完整检查清单

### 工作整理
- [ ] Sprint回顾报告已生成
- [ ] 技术债务清单已整理
- [ ] 演示材料已准备
- [ ] 文档已更新

### 版本准备
- [ ] 版本号已确定
- [ ] 版本信息已更新
- [ ] Release Notes已编写
- [ ] Changelog已更新

### 构建打包
- [ ] 清理构建环境
- [ ] Release构建成功
- [ ] 所有测试通过
- [ ] 质量检查通过
- [ ] 发布包已生成
- [ ] 文件哈希已计算

### 发布管理
- [ ] Git提交已完成
- [ ] Git标签已创建
- [ ] GitHub Release已发布
- [ ] Sprint已归档
- [ ] 临时文件已清理

### 通知分发
- [ ] 发布公告已发送
- [ ] 分发渠道已更新
- [ ] 用户文档已上线

---

## 🎯 成功标准

1. **完整性**: 所有交付物齐全
2. **质量**: 通过所有质量检查
3. **可追溯**: 版本信息完整准确
4. **可访问**: 用户能够方便下载使用
5. **可维护**: 文档和归档完善

---

## 📚 相关文档

- Sprint计划: `docs/sprints/active/sprint_X/plan.md`
- 演示文档: `docs/sprints/active/sprint_X/DEMO.md`
- 代码审查: `docs/code-review/YYYY-MM/`
- 版本管理: `docs/version-control.md`
- 构建配置: `CMakeLists.txt`

---

## 💡 最佳实践

1. **提前准备**: 在Sprint最后一天之前完成大部分工作
2. **自动化**: 尽可能自动化构建和打包流程
3. **验证充分**: 多次验证发布包的完整性
4. **文档齐全**: 确保所有文档都是最新的
5. **快速发布**: 减少从完成到发布的时间
6. **反馈循环**: 收集使用反馈用于下一个Sprint

---

**工作流维护者**: Orchestrator Agent  
**最后更新**: 2024-11-18
