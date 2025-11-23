@echo off
REM Install dependencies using vcpkg - Debug and Release versions

echo ========================================
echo   Install FlyKylin Dependencies
echo ========================================
echo.

REM Check vcpkg directory
if not exist "e:\Project\FlyKylin\vcpkg\vcpkg.exe" (
    echo [ERROR] vcpkg not found at e:\Project\FlyKylin\vcpkg
    echo Please ensure vcpkg is installed correctly.
    exit /b 1
)

cd /d e:\Project\FlyKylin\vcpkg

REM Update vcpkg baseline
echo Updating vcpkg.json baseline...
echo.

REM Install Debug versions (for development)
echo ========================================
echo   Installing Debug versions...
echo ========================================
echo.

echo Installing Protobuf (Debug)...
vcpkg install protobuf:x64-windows --triplet=x64-windows
if %errorlevel% neq 0 (
    echo [ERROR] Protobuf installation failed!
    exit /b 1
)

echo.
echo Installing GoogleTest (Debug)...
vcpkg install gtest:x64-windows --triplet=x64-windows
if %errorlevel% neq 0 (
    echo [ERROR] GoogleTest installation failed!
    exit /b 1
)

REM Integrate with MSBuild
echo.
echo Integrating vcpkg with MSBuild...
vcpkg integrate install

echo.
echo ========================================
echo   Installation Complete!
echo ========================================
echo.
echo Dependencies installed:
echo   - Protobuf (x64-windows)
echo   - GoogleTest (x64-windows)
echo.
echo Next steps:
echo   1. Run: cd e:\Project\FlyKylin
echo   2. Run: .\auto-config.cmd
echo.

pause
