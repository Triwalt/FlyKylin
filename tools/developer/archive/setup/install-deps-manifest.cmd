@echo off
REM Install dependencies using vcpkg manifest mode

echo ========================================
echo   Install FlyKylin Dependencies
echo   (Manifest Mode)
echo ========================================
echo.

cd /d e:\Project\FlyKylin

REM Check if vcpkg directory exists
if not exist "vcpkg\vcpkg.exe" (
    echo [ERROR] vcpkg not found at e:\Project\FlyKylin\vcpkg
    echo Please ensure vcpkg is installed correctly.
    pause
    exit /b 1
)

echo Using vcpkg.json manifest file...
echo Dependencies will be installed automatically.
echo.

REM Set vcpkg root
set VCPKG_ROOT=e:\Project\FlyKylin\vcpkg

REM Install dependencies defined in vcpkg.json
echo Running: vcpkg install (manifest mode)...
echo.

cd vcpkg
vcpkg install --triplet=x64-windows

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] vcpkg install failed!
    echo.
    echo Troubleshooting:
    echo   1. Check vcpkg.json is valid
    echo   2. Try: vcpkg update
    echo   3. Check internet connection
    pause
    exit /b 1
)

echo.
echo Integrating vcpkg with MSBuild...
vcpkg integrate install

echo.
echo ========================================
echo   Installation Complete!
echo ========================================
echo.
echo Dependencies installed from vcpkg.json:
echo   - Qt6 (base, network, sql, concurrent)
echo   - Protobuf
echo   - SQLite3
echo   - GoogleTest
echo.
echo Next steps:
echo   1. Clean build: rmdir /s /q build
echo   2. Reconfigure: .\auto-config.cmd
echo.

pause
