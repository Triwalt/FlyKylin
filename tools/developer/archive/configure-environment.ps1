# Configure FlyKylin Development Environment
# Sets up Qt and ONNX Runtime paths for CMake

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  FlyKylin Environment Configuration" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Qt 6.9.3 Path
$QtPath = "D:\Qt\6.9.3\msvc2022_64"
if (Test-Path $QtPath) {
    $env:CMAKE_PREFIX_PATH = $QtPath
    $env:Qt6_DIR = "$QtPath\lib\cmake\Qt6"
    $env:PATH = "$QtPath\bin;$env:PATH"
    Write-Host "[OK] Qt 6.9.3 MSVC 2022 64-bit" -ForegroundColor Green
    Write-Host "     Path: $QtPath" -ForegroundColor Gray
} else {
    Write-Host "[WARN] Qt not found at: $QtPath" -ForegroundColor Yellow
}

# ONNX Runtime C++ API
$OnnxPath = "e:\Project\FlyKylin\3rdparty\onnxruntime"
if (Test-Path "$OnnxPath\include") {
    $env:ONNXRUNTIME_DIR = $OnnxPath
    $env:PATH = "$OnnxPath\lib;$env:PATH"
    Write-Host "[OK] ONNX Runtime C++ API" -ForegroundColor Green
    Write-Host "     Path: $OnnxPath" -ForegroundColor Gray
} else {
    Write-Host "[WARN] ONNX Runtime C++ library not found" -ForegroundColor Yellow
    Write-Host "       Run: .\tools\developer\install-onnx-runtime.ps1" -ForegroundColor Yellow
}

# ONNX Models
$NsfwModel = "E:\Project\tensorflow-open_nsfw\open_nsfw.onnx"
$BgeModel = "E:\Project\tensorflow-open_nsfw\onnx_models\bge-small-zh-v1.5\bge-small-zh-v1.5.onnx"

if (Test-Path $NsfwModel) {
    Write-Host "[OK] NSFW Model" -ForegroundColor Green
    Write-Host "     Path: $NsfwModel" -ForegroundColor Gray
} else {
    Write-Host "[WARN] NSFW Model not found at: $NsfwModel" -ForegroundColor Yellow
}

if (Test-Path $BgeModel) {
    Write-Host "[OK] BGE Semantic Model" -ForegroundColor Green
    Write-Host "     Path: $BgeModel" -ForegroundColor Gray
} else {
    Write-Host "[INFO] BGE Model not found (optional)" -ForegroundColor Gray
}

# Protobuf Compiler
try {
    $null = & protoc --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        $protocVer = & protoc --version 2>&1
        Write-Host "[OK] Protobuf: $protocVer" -ForegroundColor Green
    }
} catch {
    Write-Host "[WARN] Protobuf not found" -ForegroundColor Yellow
    Write-Host "       Run: .\tools\developer\install-protobuf.ps1" -ForegroundColor Yellow
}

# CMake
try {
    $cmakeVer = & cmake --version 2>&1 | Select-Object -First 1
    Write-Host "[OK] CMake: $cmakeVer" -ForegroundColor Green
} catch {
    Write-Host "[WARN] CMake not found" -ForegroundColor Yellow
}

# MSVC Compiler
try {
    $null = Get-Command cl -ErrorAction Stop
    Write-Host "[OK] MSVC Compiler found" -ForegroundColor Green
} catch {
    Write-Host "[ERROR] MSVC not found!" -ForegroundColor Red
    Write-Host "        You MUST run this in VS Developer Command Prompt" -ForegroundColor Yellow
    Write-Host "        Run: .\tools\developer\start-vsdevcmd.ps1" -ForegroundColor Yellow
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Next Steps" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "1. Build project:" -ForegroundColor White
Write-Host "   mkdir build" -ForegroundColor Gray
Write-Host "   cd build" -ForegroundColor Gray
Write-Host "   cmake .." -ForegroundColor Gray
Write-Host "   cmake --build . --config Debug" -ForegroundColor Gray
Write-Host ""
Write-Host "Note: Environment variables are set for this session only." -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
