@echo off
REM Fix GoogleTest Runtime Library mismatch issue

echo ========================================
echo   Fix GoogleTest Debug/Release Issue
echo ========================================
echo.

echo Problem: GoogleTest Release library linked with Debug build
echo Solution: Install correct Debug version or use Release build
echo.

cd /d e:\Project\FlyKylin\vcpkg

echo Option 1: Remove and reinstall GoogleTest with Debug triplet
echo.
echo Removing existing GTest...
vcpkg remove gtest:x64-windows --recurse

echo.
echo Installing GTest Debug version...
vcpkg install gtest:x64-windows

echo.
echo Integrate vcpkg...
vcpkg integrate install

echo.
echo ========================================
echo   Fix Complete!
echo ========================================
echo.
echo Next steps:
echo   1. Clean build: rmdir /s /q e:\Project\FlyKylin\build
echo   2. Rebuild: cd e:\Project\FlyKylin
echo   3. Run: .\auto-config.cmd
echo.

pause
