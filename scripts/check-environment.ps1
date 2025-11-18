# FlyKylin Windows环境检查脚本

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " FlyKylin 开发环境检查 - Windows" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$Global:ErrorCount = 0
$Global:WarningCount = 0

function Check-Command {
    param (
        [string]$Name,
        [string]$Command,
        [string]$MinVersion = "",
        [bool]$Required = $true
    )
    
    Write-Host "检查 $Name..." -NoNewline
    
    try {
        $result = Invoke-Expression $Command 2>$null
        if ($LASTEXITCODE -eq 0 -or $result) {
            Write-Host " ✓" -ForegroundColor Green
            if ($MinVersion) {
                Write-Host "  版本: $result" -ForegroundColor Gray
            }
            return $true
        } else {
            throw "Command failed"
        }
    } catch {
        if ($Required) {
            Write-Host " ✗ 未安装" -ForegroundColor Red
            $Global:ErrorCount++
        } else {
            Write-Host " ⚠ 未安装 (可选)" -ForegroundColor Yellow
            $Global:WarningCount++
        }
        return $false
    }
}

function Check-Path {
    param (
        [string]$Name,
        [string]$PathToCheck,
        [bool]$Required = $true
    )
    
    Write-Host "检查 $Name..." -NoNewline
    
    if (Test-Path $PathToCheck) {
        Write-Host " ✓" -ForegroundColor Green
        Write-Host "  路径: $PathToCheck" -ForegroundColor Gray
        return $true
    } else {
        if ($Required) {
            Write-Host " ✗ 路径不存在" -ForegroundColor Red
            $Global:ErrorCount++
        } else {
            Write-Host " ⚠ 路径不存在 (可选)" -ForegroundColor Yellow
            $Global:WarningCount++
        }
        return $false
    }
}

Write-Host "1. 核心编译工具" -ForegroundColor Cyan
Write-Host "----------------------------------------"
Check-Command "MSVC编译器" "cl.exe 2>&1 | Select-String 'Version'" ""
Check-Command "CMake" "cmake --version" "3.20"
Check-Command "Git" "git --version"
Write-Host ""

Write-Host "2. Qt框架" -ForegroundColor Cyan
Write-Host "----------------------------------------"
$qtFound = Check-Command "qmake" "qmake --version" "" $false

if (-not $qtFound) {
    Write-Host "  提示: 请设置 Qt6_DIR 环境变量" -ForegroundColor Yellow
    if ($env:Qt6_DIR) {
        Write-Host "  当前Qt6_DIR: $env:Qt6_DIR" -ForegroundColor Gray
    }
}
Write-Host ""

Write-Host "3. 第三方库" -ForegroundColor Cyan
Write-Host "----------------------------------------"
Check-Path "ONNX Runtime" "3rdparty/onnxruntime/include" $false
Check-Path "Protobuf" "C:/vcpkg/installed/x64-windows/include/google/protobuf" $false
Write-Host ""

Write-Host "4. 代码质量工具" -ForegroundColor Cyan
Write-Host "----------------------------------------"
Check-Command "clang-format" "clang-format --version" "" $false
Check-Command "cppcheck" "cppcheck --version" "" $false
Write-Host ""

Write-Host "5. 包管理器" -ForegroundColor Cyan
Write-Host "----------------------------------------"
Check-Command "vcpkg" "vcpkg version" "" $false
Check-Command "Chocolatey" "choco --version" "" $false
Write-Host ""

Write-Host "6. Python (用于脚本和工具)" -ForegroundColor Cyan
Write-Host "----------------------------------------"
Check-Command "Python" "python --version" "3.8" $false
if (Get-Command python -ErrorAction SilentlyContinue) {
    Check-Command "pip" "pip --version" "" $false
}
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "检查完成" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($Global:ErrorCount -eq 0) {
    Write-Host "✓ 所有必需组件已安装!" -ForegroundColor Green
} else {
    Write-Host "✗ 发现 $Global:ErrorCount 个必需组件未安装" -ForegroundColor Red
    Write-Host "请参考 docs/环境配置指南.md 进行安装" -ForegroundColor Yellow
}

if ($Global:WarningCount -gt 0) {
    Write-Host "⚠ $Global:WarningCount 个可选组件未安装" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "详细配置指南: docs/环境配置指南.md" -ForegroundColor Cyan
Write-Host ""

# 返回退出码
if ($Global:ErrorCount -gt 0) {
    exit 1
} else {
    exit 0
}
