# 诊断测试程序DLL依赖问题
$ErrorActionPreference = "Continue"

Write-Host "========================================"
Write-Host " FlyKylin Test Dependency Diagnostic"
Write-Host "========================================"

$testExe = "build\windows-release\bin\Release\flykylin_tests.exe"
$testDir = "build\windows-release\bin\Release"

if (!(Test-Path $testExe)) {
    Write-Host "[ERROR] Test executable not found!" -ForegroundColor Red
    exit 1
}

Write-Host "`n[1] Checking test executable..." -ForegroundColor Cyan
$fileInfo = Get-Item $testExe
Write-Host "  Size: $($fileInfo.Length) bytes"
Write-Host "  Modified: $($fileInfo.LastWriteTime)"

Write-Host "`n[2] Available DLLs in Release folder:" -ForegroundColor Cyan
Get-ChildItem $testDir -Filter "*.dll" | Format-Table Name, Length, LastWriteTime -AutoSize

Write-Host "`n[3] Checking vcpkg DLL folder:" -ForegroundColor Cyan
$vcpkgBin = "build\windows-release\vcpkg_installed\x64-windows\bin"
if (Test-Path $vcpkgBin) {
    Write-Host "  vcpkg bin folder exists"
    Get-ChildItem $vcpkgBin -Filter "*.dll" | Select-Object Name | Format-Table -AutoSize
} else {
    Write-Host "  [WARNING] vcpkg bin folder not found" -ForegroundColor Yellow
}

Write-Host "`n[4] Attempting to run with verbose error..." -ForegroundColor Cyan
Push-Location $testDir
Write-Host "  Working directory: $(Get-Location)"
Write-Host "  Command: .\flykylin_tests.exe --help"

# 尝试运行并捕获所有输出
$process = Start-Process -FilePath ".\flykylin_tests.exe" -ArgumentList "--help" -Wait -PassThru -NoNewWindow
$exitCode = $process.ExitCode

Write-Host "  Exit code: $exitCode (Hex: 0x$($exitCode.ToString('X')))"

# 解释错误码
if ($exitCode -eq -1073741511) {
    Write-Host "`n[DIAGNOSIS]" -ForegroundColor Yellow
    Write-Host "  Error 0xC0000139 = STATUS_ENTRYPOINT_NOT_FOUND"
    Write-Host "  This typically means a DLL is found but missing an expected function."
    Write-Host "  Possible causes:"
    Write-Host "  - GoogleTest DLL version mismatch"
    Write-Host "  - Mixing static and dynamic CRT"
    Write-Host "  - Wrong protobuf DLL version"
}

Pop-Location

Write-Host "`n[5] Checking if main program works..." -ForegroundColor Cyan
$mainExe = "FlyKylin.exe"
if (Test-Path $mainExe) {
    Write-Host "  Found FlyKylin.exe, attempting quick start/stop..."
    $mainProcess = Start-Process -FilePath ".\FlyKylin.exe" -PassThru -WindowStyle Hidden
    Start-Sleep -Milliseconds 500
    if (!$mainProcess.HasExited) {
        $mainProcess.Kill()
        Write-Host "  [OK] FlyKylin.exe started successfully" -ForegroundColor Green
    }
}

Write-Host "`n========================================"
Write-Host "Diagnostic complete. Check output above for clues."
Write-Host "========================================"
