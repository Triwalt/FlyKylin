@echo off
REM FlyKylin Complete Environment Setup
REM Automatically installs all dependencies and configures the project

setlocal EnableDelayedExpansion

echo ========================================
echo   FlyKylin Auto Setup
echo ========================================
echo.
echo This will automatically:
echo   1. Install vcpkg (if needed)
echo   2. Install Protobuf C++ libraries
echo   3. Download ONNX Runtime (if needed)
echo   4. Configure CMake with MSVC
echo   5. Build the project
echo.
echo This may take 10-15 minutes...
echo.
pause

REM Step 1: Install vcpkg
echo.
echo [1/5] Setting up vcpkg...
echo ========================================
if not exist "C:\vcpkg" (
    echo vcpkg not found, installing...
    cd C:\
    git clone https://github.com/microsoft/vcpkg
    if !errorlevel! neq 0 (
        echo [ERROR] Failed to clone vcpkg
        echo Please install git first: https://git-scm.com/
        pause
        exit /b 1
    )
    
    cd vcpkg
    call bootstrap-vcpkg.bat
    if !errorlevel! neq 0 (
        echo [ERROR] vcpkg bootstrap failed
        pause
        exit /b 1
    )
    echo [OK] vcpkg installed
) else (
    echo [OK] vcpkg already installed
)

REM Step 2: Install Protobuf
echo.
echo [2/5] Installing Protobuf C++ libraries...
echo ========================================
C:\vcpkg\vcpkg list | findstr "protobuf:x64-windows" >nul
if !errorlevel! neq 0 (
    echo Installing protobuf:x64-windows...
    C:\vcpkg\vcpkg install protobuf:x64-windows
    if !errorlevel! neq 0 (
        echo [ERROR] Protobuf installation failed
        pause
        exit /b 1
    )
    echo [OK] Protobuf installed
) else (
    echo [OK] Protobuf already installed
)

REM Step 3: Download ONNX Runtime
echo.
echo [3/5] Checking ONNX Runtime...
echo ========================================
cd /d e:\Project\FlyKylin
if not exist "3rdparty\onnxruntime\include" (
    echo Downloading ONNX Runtime 1.23.2...
    
    set ONNX_URL=https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-win-x64-1.23.2.zip
    set TEMP_ZIP=%TEMP%\onnxruntime.zip
    
    powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $webClient = New-Object System.Net.WebClient; $webClient.DownloadFile('!ONNX_URL!', '!TEMP_ZIP!')}"
    
    if !errorlevel! neq 0 (
        echo [WARN] Auto-download failed
        echo Please manually download from:
        echo https://github.com/microsoft/onnxruntime/releases/tag/v1.23.2
        echo File: onnxruntime-win-x64-1.23.2.zip
        echo Extract to: e:\Project\FlyKylin\3rdparty\onnxruntime
        pause
    ) else (
        echo Extracting...
        if not exist "3rdparty" mkdir 3rdparty
        powershell -Command "Expand-Archive -Path '!TEMP_ZIP!' -DestinationPath '3rdparty' -Force"
        
        if exist "3rdparty\onnxruntime-win-x64-1.23.2" (
            if exist "3rdparty\onnxruntime" rmdir /s /q "3rdparty\onnxruntime"
            ren "3rdparty\onnxruntime-win-x64-1.23.2" "onnxruntime"
        )
        
        del /f "!TEMP_ZIP!"
        echo [OK] ONNX Runtime downloaded and extracted
    )
) else (
    echo [OK] ONNX Runtime already installed
)

REM Step 4: Clean and configure CMake
echo.
echo [4/5] Configuring CMake...
echo ========================================
if exist build (
    echo Cleaning old build...
    rmdir /s /q build 2>nul
    timeout /t 1 /nobreak >nul
)
mkdir build
cd build

echo Configuring with Visual Studio 2022...
cmake -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DCMAKE_PREFIX_PATH=D:/Qt/6.9.3/msvc2022_64 ^
    -DQt6_DIR=D:/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6 ^
    ..

if !errorlevel! neq 0 (
    echo.
    echo [ERROR] CMake configuration failed!
    echo.
    echo Please check:
    echo   1. Visual Studio 2022 is installed
    echo   2. Qt is at D:\Qt\6.9.3\msvc2022_64
    echo   3. All dependencies are installed
    echo.
    cd ..
    pause
    exit /b 1
)

echo [OK] CMake configured successfully

REM Step 5: Build
echo.
echo [5/5] Building project...
echo ========================================
cmake --build . --config Debug

if !errorlevel! neq 0 (
    echo.
    echo [ERROR] Build failed!
    echo Check the error messages above
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo   Setup Complete!
echo ========================================
echo.
echo Executable: build\bin\Debug\FlyKylin.exe
echo.
echo Next steps:
echo   cd build
echo   cmake --build . --config Release
echo.
pause
