@echo off
REM Install Protobuf with C++ libraries using vcpkg

echo ========================================
echo   Install Protobuf via vcpkg
echo ========================================
echo.

REM Check if vcpkg exists
if not exist "C:\vcpkg" (
    echo vcpkg not found, installing...
    echo.
    
    cd C:\
    git clone https://github.com/microsoft/vcpkg
    cd vcpkg
    call bootstrap-vcpkg.bat
    
    echo.
    echo vcpkg installed successfully
    echo.
)

echo Installing Protobuf for x64-windows...
echo This may take 5-10 minutes...
echo.

C:\vcpkg\vcpkg install protobuf:x64-windows

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Installation failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo   Protobuf installed successfully!
echo ========================================
echo.
echo vcpkg root: C:\vcpkg
echo.
echo Next: Use this CMake command:
echo   cmake -G "Visual Studio 17 2022" -A x64 ^
echo     -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
echo     -DCMAKE_PREFIX_PATH=D:/Qt/6.9.3/msvc2022_64 ^
echo     ..
echo.
pause
