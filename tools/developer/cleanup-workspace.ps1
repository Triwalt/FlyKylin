#!/usr/bin/env pwsh
# ============================================
# 工作区清理和整理脚本
# ============================================
# 用途：整理根目录散落的脚本和文档，规范化项目结构

param(
    [switch]$DryRun
)

$ProjectRoot = "e:\Project\FlyKylin"
Set-Location $ProjectRoot

Write-Host @"
╔════════════════════════════════════════════════╗
║          FlyKylin 工作区整理工具               ║
╚════════════════════════════════════════════════╝
"@ -ForegroundColor Cyan

function Move-FileIfExists {
    param(
        [string]$Source,
        [string]$Destination,
        [string]$Description
    )
    
    if (Test-Path $Source) {
        $destDir = Split-Path $Destination -Parent
        if (!(Test-Path $destDir)) {
            Write-Host "  创建目录: $destDir" -ForegroundColor Gray
            if (!$DryRun) {
                New-Item -ItemType Directory -Force -Path $destDir | Out-Null
            }
        }
        
        Write-Host "  移动: $Source → $Destination" -ForegroundColor Yellow
        Write-Host "        ($Description)" -ForegroundColor Gray
        
        if (!$DryRun) {
            Move-Item -Path $Source -Destination $Destination -Force
        }
    } else {
        Write-Host "  跳过: $Source (不存在)" -ForegroundColor DarkGray
    }
}

# ============================================
# 1. 整理构建脚本
# ============================================
Write-Host "`n[1/4] 整理构建脚本..." -ForegroundColor Green

$buildScripts = @(
    @{Src="build-release.cmd"; Dest="tools/developer/build/build-release.cmd"; Desc="Release构建脚本"},
    @{Src="build-release-only.cmd"; Dest="tools/developer/build/build-release-only.cmd"; Desc="仅构建（不配置）"},
    @{Src="build-msvc.cmd"; Dest="tools/developer/build/build-msvc.cmd"; Desc="MSVC构建脚本"},
    @{Src="configure-cmake.cmd"; Dest="tools/developer/build/configure-cmake.cmd"; Desc="CMake配置脚本"},
    @{Src="auto-config.cmd"; Dest="tools/developer/build/auto-config.cmd"; Desc="自动配置脚本"}
)

foreach ($script in $buildScripts) {
    Move-FileIfExists -Source $script.Src -Destination $script.Dest -Description $script.Desc
}

# ============================================
# 2. 整理安装和设置脚本
# ============================================
Write-Host "`n[2/4] 整理安装和设置脚本..." -ForegroundColor Green

$setupScripts = @(
    @{Src="install-dependencies.cmd"; Dest="tools/developer/setup/install-dependencies.cmd"; Desc="依赖安装脚本"},
    @{Src="install-deps-manifest.cmd"; Dest="tools/developer/setup/install-deps-manifest.cmd"; Desc="清单依赖安装"},
    @{Src="install-protobuf-vcpkg.cmd"; Dest="tools/developer/setup/install-protobuf-vcpkg.cmd"; Desc="Protobuf安装"},
    @{Src="setup-all.cmd"; Dest="tools/developer/setup/setup-all.cmd"; Desc="完整环境设置"},
    @{Src="setup-simplified.cmd"; Dest="tools/developer/setup/setup-simplified.cmd"; Desc="简化设置脚本"},
    @{Src="fix-gtest-anaconda.cmd"; Dest="tools/developer/setup/fix-gtest-anaconda.cmd"; Desc="修复GoogleTest问题"},
    @{Src="fix-gtest-issue.cmd"; Dest="tools/developer/setup/fix-gtest-issue.cmd"; Desc="修复GoogleTest问题"}
)

foreach ($script in $setupScripts) {
    Move-FileIfExists -Source $script.Src -Destination $script.Dest -Description $script.Desc
}

# ============================================
# 3. 整理测试和运行脚本
# ============================================
Write-Host "`n[3/4] 整理测试和运行脚本..." -ForegroundColor Green

$testScripts = @(
    @{Src="run-tests.cmd"; Dest="tools/developer/test/run-tests.cmd"; Desc="运行测试"},
    @{Src="test-dual-instance.cmd"; Dest="tools/developer/test/test-dual-instance.cmd"; Desc="双实例测试"},
    @{Src="run-with-qt.cmd"; Dest="tools/developer/run/run-with-qt.cmd"; Desc="运行应用（带Qt环境）"},
    @{Src="deploy-and-run.cmd"; Dest="tools/developer/run/deploy-and-run.cmd"; Desc="部署并运行"}
)

foreach ($script in $testScripts) {
    Move-FileIfExists -Source $script.Src -Destination $script.Dest -Description $script.Desc
}

# ============================================
# 4. 整理Sprint文档
# ============================================
Write-Host "`n[4/4] 整理Sprint文档..." -ForegroundColor Green

# 移动Sprint 0文档到归档
$sprint0Docs = @(
    @{Src="SPRINT0_COMPLETE.md"; Dest="docs/sprints/archive/sprint_0/COMPLETE.md"; Desc="Sprint 0完成报告"},
    @{Src="SPRINT0_STATUS.md"; Dest="docs/sprints/archive/sprint_0/STATUS.md"; Desc="Sprint 0状态报告"},
    @{Src="BUILD_SUCCESS.md"; Dest="docs/sprints/archive/sprint_0/BUILD_SUCCESS.md"; Desc="构建成功记录"}
)

foreach ($doc in $sprint0Docs) {
    Move-FileIfExists -Source $doc.Src -Destination $doc.Dest -Description $doc.Desc
}

# 移动Sprint 1进度文档
Move-FileIfExists -Source "SPRINT1_PROGRESS.md" `
    -Destination "docs/sprints/active/sprint_1/PROGRESS.md" `
    -Description "Sprint 1进度报告"

# 移动agent_prompts.yaml
Move-FileIfExists -Source "agent_prompts.yaml" `
    -Destination ".windsurf/agent_prompts.yaml" `
    -Description "Agent提示词配置"

# ============================================
# 完成
# ============================================
Write-Host @"

╔════════════════════════════════════════════════╗
║              🎉 整理完成！                      ║
╚════════════════════════════════════════════════╝

目录结构：
  tools/developer/
    ├── build/         构建相关脚本
    ├── setup/         环境配置脚本
    ├── test/          测试脚本
    └── run/           运行脚本

  docs/sprints/
    ├── archive/       已完成Sprint归档
    └── active/        进行中Sprint

  .windsurf/
    └── agent_prompts.yaml  Agent配置

"@ -ForegroundColor Green

if ($DryRun) {
    Write-Host "这是模拟运行，没有实际移动文件。" -ForegroundColor Yellow
    Write-Host "执行实际清理：.\tools\developer\cleanup-workspace.ps1" -ForegroundColor Yellow
}
