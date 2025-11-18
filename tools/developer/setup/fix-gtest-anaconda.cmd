@echo off
REM Fix GoogleTest issue by removing Anaconda from PATH temporarily

echo ========================================
echo   Fix GoogleTest PATH Issue
echo ========================================
echo.

echo Problem: CMake finds Anaconda's GTest (Release only)
echo Solution: Temporarily remove Anaconda from PATH and rebuild
echo.

cd /d e:\Project\FlyKylin

REM Save current PATH
set ORIGINAL_PATH=%PATH%

REM Remove Anaconda from PATH
set PATH=%PATH:D:/Anaconda/Library/lib/cmake/GTest;=%
set PATH=%PATH:D:/Anaconda/Library/bin;=%
set PATH=%PATH:D:/Anaconda/Scripts;=%
set PATH=%PATH:D:/Anaconda;=%

echo Anaconda temporarily removed from PATH
echo.

REM Clean build
if exist build (
    echo Cleaning build directory...
    rmdir /s /q build
)

echo.
echo Configuring CMake without Anaconda GTest...
echo.

mkdir build
cd build

cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_PREFIX_PATH=E:/Qt/6.9.3/msvc2022_64 ^
    -DCMAKE_TOOLCHAIN_FILE=E:/Project/FlyKylin/vcpkg/scripts/buildsystems/vcpkg.cmake

if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    set PATH=%ORIGINAL_PATH%
    pause
    exit /b 1
)

echo.
echo Building...
cmake --build . --config Debug

if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    set PATH=%ORIGINAL_PATH%
    pause
    exit /b 1
)

REM Restore PATH
set PATH=%ORIGINAL_PATH%

echo.
echo ========================================
echo   Build Success!
echo ========================================
echo.
echo Executable: build\bin\Debug\FlyKylin.exe
echo Tests: build\bin\Debug\flykylin_tests.exe
echo.
echo Run tests:
echo   cd build
echo   ctest -C Debug --output-on-failure
echo.

pause
