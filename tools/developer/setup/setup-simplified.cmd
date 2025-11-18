@echo off
REM Simplified setup without vcpkg dependency

setlocal EnableDelayedExpansion

echo ========================================
echo   FlyKylin Simplified Setup
echo ========================================
echo.

REM Step 1: Check ONNX Runtime
echo [1/3] Checking ONNX Runtime...
echo ========================================
cd /d e:\Project\FlyKylin
if not exist "3rdparty\onnxruntime\include" (
    echo ONNX Runtime not found
    echo Please manually download:
    echo   URL: https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-win-x64-1.23.2.zip
    echo   Extract to: e:\Project\FlyKylin\3rdparty\onnxruntime
    echo.
    set ONNX_MISSING=1
) else (
    echo [OK] ONNX Runtime found
    set ONNX_MISSING=0
)

REM Step 2: Use system Protobuf
echo.
echo [2/3] Checking Protobuf...
echo ========================================
where protoc >nul 2>&1
if !errorlevel! equ 0 (
    for /f "tokens=*" %%i in ('where protoc') do set PROTOC_PATH=%%i
    echo [OK] Found protoc: !PROTOC_PATH!
) else (
    echo [WARN] protoc not found in PATH
)

REM Step 3: Try to configure without Protobuf for now
echo.
echo [3/3] Configuring CMake (without Protobuf)...
echo ========================================

if !ONNX_MISSING! equ 1 (
    echo Cannot proceed without ONNX Runtime
    pause
    exit /b 1
)

if exist build rmdir /s /q build 2>nul
mkdir build
cd build

REM Try minimal configuration
cmake -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_PREFIX_PATH=D:/Qt/6.9.3/msvc2022_64 ^
    -DQt6_DIR=D:/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6 ^
    ..

if !errorlevel! neq 0 (
    echo.
    echo [ERROR] Configuration failed
    cd ..
    pause
    exit /b 1
)

echo.
echo [OK] CMake configured
echo.
echo Now trying to build...
cmake --build . --config Debug

cd ..
pause
