#!/usr/bin/env pwsh
# ============================================
# Local CI Build Test Script
# ============================================
# Purpose: Test the build process locally before pushing to GitHub
# Usage: .\tools\developer\test-ci-build.ps1 [-Platform windows|linux]

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("windows", "linux", "all")]
    [string]$Platform = "windows",
    
    [switch]$SkipTests,
    [switch]$SkipPackage
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
Set-Location $ProjectRoot

Write-Host @"
╔════════════════════════════════════════════════╗
║          Local CI Build Test                   ║
║          Platform: $Platform                        ║
╚════════════════════════════════════════════════╝
"@ -ForegroundColor Cyan

function Test-WindowsBuild {
    Write-Host "`n[Windows Build Test]" -ForegroundColor Yellow
    
    # Check prerequisites
    Write-Host "Checking prerequisites..." -ForegroundColor Gray
    
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if (!$cmake) {
        Write-Host "  ✗ CMake not found" -ForegroundColor Red
        Write-Host "    Install: https://cmake.org/download/" -ForegroundColor Gray
        return $false
    }
    Write-Host "  ✓ CMake: $($cmake.Version)" -ForegroundColor Green
    
    $msbuild = Get-Command msbuild -ErrorAction SilentlyContinue
    if (!$msbuild) {
        Write-Host "  ✗ MSBuild not found" -ForegroundColor Red
        Write-Host "    Install Visual Studio 2022" -ForegroundColor Gray
        return $false
    }
    Write-Host "  ✓ MSBuild found" -ForegroundColor Green
    
    if (!(Test-Path "D:\Qt\6.9.3\msvc2022_64")) {
        Write-Host "  ⚠ Qt not found at D:\Qt\6.9.3\msvc2022_64" -ForegroundColor Yellow
        Write-Host "    Update CMAKE_PREFIX_PATH in script if Qt is elsewhere" -ForegroundColor Gray
    } else {
        Write-Host "  ✓ Qt found" -ForegroundColor Green
    }
    
    # Clean build directory
    Write-Host "`nCleaning build directory..." -ForegroundColor Gray
    if (Test-Path "build-ci-test") {
        Remove-Item -Recurse -Force "build-ci-test"
    }
    
    # Configure
    Write-Host "`nConfiguring CMake..." -ForegroundColor Gray
    $configCmd = "cmake -B build-ci-test -G `"Visual Studio 17 2022`" -A x64 " +
                 "-DCMAKE_BUILD_TYPE=Release " +
                 "-DCMAKE_PREFIX_PATH=`"D:/Qt/6.9.3/msvc2022_64`""
    
    if (Test-Path "vcpkg\scripts\buildsystems\vcpkg.cmake") {
        $configCmd += " -DCMAKE_TOOLCHAIN_FILE=`"$ProjectRoot\vcpkg\scripts\buildsystems\vcpkg.cmake`""
    }
    
    Invoke-Expression $configCmd
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ✗ CMake configuration failed" -ForegroundColor Red
        return $false
    }
    Write-Host "  ✓ Configuration successful" -ForegroundColor Green
    
    # Build
    Write-Host "`nBuilding..." -ForegroundColor Gray
    cmake --build build-ci-test --config Release -j4
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ✗ Build failed" -ForegroundColor Red
        return $false
    }
    Write-Host "  ✓ Build successful" -ForegroundColor Green
    
    # Test
    if (!$SkipTests) {
        Write-Host "`nRunning tests..." -ForegroundColor Gray
        Push-Location build-ci-test
        ctest -C Release --output-on-failure
        $testResult = $LASTEXITCODE
        Pop-Location
        
        if ($testResult -ne 0) {
            Write-Host "  ✗ Tests failed" -ForegroundColor Red
            return $false
        }
        Write-Host "  ✓ All tests passed" -ForegroundColor Green
    }
    
    # Package
    if (!$SkipPackage) {
        Write-Host "`nCreating package..." -ForegroundColor Gray
        
        $packageDir = "build-ci-test\package\FlyKylin-test-windows-amd64"
        New-Item -ItemType Directory -Force -Path $packageDir | Out-Null
        
        Copy-Item "build-ci-test\bin\Release\FlyKylin.exe" -Destination "$packageDir\"
        
        # Try to run windeployqt if available
        $windeployqt = "D:\Qt\6.9.3\msvc2022_64\bin\windeployqt.exe"
        if (Test-Path $windeployqt) {
            & $windeployqt --release --no-translations "$packageDir\FlyKylin.exe"
            Write-Host "  ✓ Qt dependencies deployed" -ForegroundColor Green
        } else {
            Write-Host "  ⚠ windeployqt not found, skipping Qt deployment" -ForegroundColor Yellow
        }
        
        Write-Host "  ✓ Package created: $packageDir" -ForegroundColor Green
    }
    
    return $true
}

function Test-LinuxBuild {
    Write-Host "`n[Linux Build Test]" -ForegroundColor Yellow
    Write-Host "  ⚠ Linux build test requires WSL or Linux environment" -ForegroundColor Yellow
    
    # Check if WSL is available
    $wsl = Get-Command wsl -ErrorAction SilentlyContinue
    if (!$wsl) {
        Write-Host "  ✗ WSL not found" -ForegroundColor Red
        Write-Host "    Install WSL to test Linux builds on Windows" -ForegroundColor Gray
        return $false
    }
    
    Write-Host "  ✓ WSL found, running Linux build test..." -ForegroundColor Green
    
    # Run build in WSL
    $wslScript = @'
#!/bin/bash
set -e

echo "Installing dependencies..."
sudo apt-get update -qq
sudo apt-get install -y build-essential cmake ninja-build qt6-base-dev

echo "Configuring CMake..."
cmake -B build-ci-test -G Ninja -DCMAKE_BUILD_TYPE=Release

echo "Building..."
cmake --build build-ci-test -j$(nproc)

echo "Running tests..."
cd build-ci-test && ctest --output-on-failure

echo "✓ Linux build test successful"
'@
    
    $wslScript | wsl bash
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ✗ Linux build failed" -ForegroundColor Red
        return $false
    }
    
    Write-Host "  ✓ Linux build successful" -ForegroundColor Green
    return $true
}

# Main execution
$success = $true

if ($Platform -eq "windows" -or $Platform -eq "all") {
    if (!(Test-WindowsBuild)) {
        $success = $false
    }
}

if ($Platform -eq "linux" -or $Platform -eq "all") {
    if (!(Test-LinuxBuild)) {
        $success = $false
    }
}

# Summary
Write-Host "`n" -NoNewline
if ($success) {
    Write-Host @"
╔════════════════════════════════════════════════╗
║          ✓ All Tests Passed!                   ║
╚════════════════════════════════════════════════╝

Your build is ready for CI/CD!

Next steps:
1. Commit your changes
2. Push to GitHub
3. GitHub Actions will build all platforms automatically

To trigger a release:
  git tag -a v0.1.0 -m "Release v0.1.0"
  git push origin v0.1.0

"@ -ForegroundColor Green
    exit 0
} else {
    Write-Host @"
╔════════════════════════════════════════════════╗
║          ✗ Tests Failed                        ║
╚════════════════════════════════════════════════╝

Please fix the errors before pushing to GitHub.

Check the error messages above for details.

"@ -ForegroundColor Red
    exit 1
}
