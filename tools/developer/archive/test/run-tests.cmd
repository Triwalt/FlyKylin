@echo off
REM Run FlyKylin Tests

echo ========================================
echo   Run FlyKylin Tests (Release)
echo ========================================
echo.

REM Add Qt to PATH
set PATH=D:\Qt\6.9.3\msvc2022_64\bin;%PATH%

cd /d e:\Project\FlyKylin\build

echo Running tests with ctest...
echo.

ctest -C Release --output-on-failure

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo   All Tests PASSED!
    echo ========================================
) else (
    echo.
    echo ========================================
    echo   Some Tests FAILED!
    echo ========================================
)

echo.
pause
