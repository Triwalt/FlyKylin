@echo off
REM Run FlyKylin with Qt 6.9.3 in PATH

echo ========================================
echo   Run FlyKylin with Qt 6.9.3
echo ========================================
echo.

REM Add Qt to PATH temporarily
set PATH=D:\Qt\6.9.3\msvc2022_64\bin;%PATH%

cd /d e:\Project\FlyKylin\build\bin\Release

echo Running FlyKylin.exe...
echo.

FlyKylin.exe

pause
