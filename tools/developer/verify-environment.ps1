# Verify FlyKylin Development Environment
# Checks all required tools and dependencies

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  FlyKylin Environment Verification" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$allGood = $true

# Check Qt
Write-Host "`n[Qt 6.9.3]" -ForegroundColor Yellow
if (Test-Path "D:\Qt\6.9.3\msvc2022_64") {
    Write-Host "  [OK] Qt 6.9.3 MSVC 2022 64-bit" -ForegroundColor Green
    Write-Host "       Path: D:\Qt\6.9.3\msvc2022_64" -ForegroundColor Gray
} else {
    Write-Host "  [FAIL] Qt not found" -ForegroundColor Red
    $allGood = $false
}

# Check ONNX Runtime
Write-Host "`n[ONNX Runtime]" -ForegroundColor Yellow
if (Test-Path "e:\Project\FlyKylin\3rdparty\onnxruntime\include") {
    Write-Host "  [OK] ONNX Runtime C++ API" -ForegroundColor Green
    $dllPath = "e:\Project\FlyKylin\3rdparty\onnxruntime\lib\onnxruntime.dll"
    if (Test-Path $dllPath) {
        $dllSize = (Get-Item $dllPath).Length / 1MB
        Write-Host "       DLL size: $([math]::Round($dllSize, 2)) MB" -ForegroundColor Gray
    }
} else {
    Write-Host "  [FAIL] ONNX Runtime not installed" -ForegroundColor Red
    Write-Host "       Run: .\tools\developer\install-onnx-runtime.ps1" -ForegroundColor Yellow
    $allGood = $false
}

# Check Protobuf
Write-Host "`n[Protobuf]" -ForegroundColor Yellow
try {
    $protocVersion = & protoc --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  [OK] $protocVersion" -ForegroundColor Green
    } else {
        throw "protoc failed"
    }
} catch {
    Write-Host "  [FAIL] Protobuf not installed" -ForegroundColor Red
    Write-Host "       Run: .\tools\developer\install-protobuf.ps1" -ForegroundColor Yellow
    $allGood = $false
}

# Check CMake
Write-Host "`n[CMake]" -ForegroundColor Yellow
try {
    $cmakeVersion = & cmake --version 2>&1 | Select-Object -First 1
    Write-Host "  [OK] $cmakeVersion" -ForegroundColor Green
} catch {
    Write-Host "  [FAIL] CMake not installed" -ForegroundColor Red
    $allGood = $false
}

# Check MSVC
Write-Host "`n[MSVC Compiler]" -ForegroundColor Yellow
try {
    $cl = Get-Command cl -ErrorAction Stop
    Write-Host "  [OK] MSVC found" -ForegroundColor Green
} catch {
    Write-Host "  [WARN] MSVC not in PATH" -ForegroundColor Yellow
    Write-Host "       Please open VS Developer Command Prompt" -ForegroundColor Gray
}

# Check AI Models
Write-Host "`n[AI Models]" -ForegroundColor Yellow
$nsfwModel = "E:\Project\tensorflow-open_nsfw\open_nsfw.onnx"
if (Test-Path $nsfwModel) {
    Write-Host "  [OK] NSFW Model" -ForegroundColor Green
    Write-Host "       Path: $nsfwModel" -ForegroundColor Gray
} else {
    Write-Host "  [WARN] NSFW Model not found" -ForegroundColor Yellow
    $allGood = $false
}

$bgeModel = "E:\Project\tensorflow-open_nsfw\onnx_models\bge-small-zh-v1.5\bge-small-zh-v1.5.onnx"
if (Test-Path $bgeModel) {
    Write-Host "  [OK] BGE Semantic Model" -ForegroundColor Green
    Write-Host "       Path: $bgeModel" -ForegroundColor Gray
} else {
    Write-Host "  [WARN] BGE Model not found" -ForegroundColor Yellow
}

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
if ($allGood) {
    Write-Host "  All Required Dependencies Ready!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Next Steps:" -ForegroundColor White
    Write-Host "  1. Open VS Developer Command Prompt" -ForegroundColor Gray
    Write-Host "  2. cd e:\Project\FlyKylin" -ForegroundColor Gray
    Write-Host "  3. .\configure-environment.ps1" -ForegroundColor Gray
    Write-Host "  4. mkdir build && cd build" -ForegroundColor Gray
    Write-Host "  5. cmake .." -ForegroundColor Gray
    Write-Host "  6. cmake --build . --config Debug" -ForegroundColor Gray
} else {
    Write-Host "  Some Dependencies Missing" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Install missing dependencies:" -ForegroundColor White
    Write-Host "  .\tools\developer\install-onnx-runtime.ps1" -ForegroundColor Gray
    Write-Host "  .\tools\developer\install-protobuf.ps1" -ForegroundColor Gray
}

Write-Host ""
