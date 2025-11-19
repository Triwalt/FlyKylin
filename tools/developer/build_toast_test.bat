@echo off
REM Windows Toast notification test tool compile script

echo ========================================
echo   Toast
echo ========================================
echo.

REM check msvc
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] NO MSVC
    echo RUN "Developer Command Prompt for VS 2022"
    pause
    exit /b 1
)

echo [INFO] compiling...
cl /EHsc /std:c++17 test_windows_toast.cpp /link windowsapp.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] COMPILED
    echo.
    echo RUN:
    echo   test_windows_toast.exe
    echo.
) else (
    echo.
    echo [ERROR] COMPILE FAILED
    exit /b 1
)

pause
