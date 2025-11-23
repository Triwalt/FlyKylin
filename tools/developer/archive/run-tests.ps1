# 运行单元测试
$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Running FlyKylin Unit Tests" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 设置工作目录为测试可执行文件所在目录
$testExe = "build\windows-release\bin\Release\flykylin_tests.exe"
$testDir = "build\windows-release\bin\Release"

if (!(Test-Path $testExe)) {
    Write-Host "[ERROR] Test executable not found: $testExe" -ForegroundColor Red
    exit 1
}

Write-Host "[INFO] Test executable: $testExe" -ForegroundColor Green
Write-Host "[INFO] Working directory: $testDir" -ForegroundColor Green

# 列出DLL
Write-Host "`n[INFO] Available DLLs:" -ForegroundColor Yellow
Get-ChildItem $testDir -Filter "*.dll" | Select-Object Name | Format-Table -AutoSize

# 尝试运行测试
Write-Host "`n[INFO] Running tests..." -ForegroundColor Green
Push-Location $testDir
try {
    & ".\flykylin_tests.exe" @args
    $exitCode = $LASTEXITCODE
    if ($exitCode -eq 0) {
        Write-Host "`n[SUCCESS] All tests passed!" -ForegroundColor Green
    } else {
        Write-Host "`n[ERROR] Tests failed with exit code: $exitCode" -ForegroundColor Red
    }
    exit $exitCode
} catch {
    Write-Host "`n[ERROR] Exception: $_" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location
}
