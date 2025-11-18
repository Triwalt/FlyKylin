@echo off
REM Deploy Qt DLLs and run FlyKylin

echo ========================================
echo   Deploy Qt Dependencies
echo ========================================
echo.

cd /d e:\Project\FlyKylin\build\bin\Release

REM Deploy Qt DLLs
echo Deploying Qt 6.9.3 DLLs...
D:\Qt\6.9.3\msvc2022_64\bin\windeployqt.exe FlyKylin.exe --release --no-translations

if %errorlevel% neq 0 (
    echo [ERROR] windeployqt failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo   Qt Deployment Complete!
echo ========================================
echo.
echo Running FlyKylin.exe...
echo.

FlyKylin.exe

pause
