#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Verify Protobuf integration and ProtobufSerializer implementation
.DESCRIPTION
    Check CMake configuration, Protobuf generated files, compilation, and tests
.EXAMPLE
    .\tools\developer\verify-protobuf-integration.ps1
#>

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Protobuf Integration Verification" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. Check vcpkg is present
Write-Host "[1/6] Checking vcpkg setup..." -ForegroundColor Yellow
$vcpkgExe = "$PSScriptRoot\..\..\vcpkg\vcpkg.exe"
if (Test-Path $vcpkgExe) {
    Write-Host "  [OK] vcpkg.exe found" -ForegroundColor Green
    Write-Host "  Note: Dependencies will be installed automatically by CMake" -ForegroundColor Gray
} else {
    Write-Host "  [ERROR] vcpkg.exe not found at: $vcpkgExe" -ForegroundColor Red
    Write-Host "  Please bootstrap vcpkg: .\vcpkg\bootstrap-vcpkg.bat" -ForegroundColor Yellow
    exit 1
}

# 2. CMake configuration
Write-Host ""
Write-Host "[2/6] Configuring CMake..." -ForegroundColor Yellow
Write-Host "  Command: cmake --preset windows-release" -ForegroundColor Gray

try {
    cmake --preset windows-release 2>&1 | Tee-Object -Variable cmakeOutput
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [ERROR] CMake configuration failed" -ForegroundColor Red
        exit 1
    }
    
    # Check if Protobuf was found
    if ($cmakeOutput -match "Protobuf found") {
        Write-Host "  [OK] CMake configured successfully, Protobuf found" -ForegroundColor Green
    } else {
        Write-Host "  [WARN] CMake configured, but Protobuf not found" -ForegroundColor Yellow
    }
} catch {
    Write-Host "  [ERROR] CMake exception: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# 3. Check generated Protobuf files
Write-Host ""
Write-Host "[3/6] Checking generated Protobuf files..." -ForegroundColor Yellow

$buildDir = "build\windows-release"
$pbHeader = "$buildDir\messages.pb.h"
$pbSource = "$buildDir\messages.pb.cc"

if ((Test-Path $pbHeader) -and (Test-Path $pbSource)) {
    Write-Host "  [OK] Protobuf files generated:" -ForegroundColor Green
    Write-Host "    - $pbHeader" -ForegroundColor Gray
    Write-Host "    - $pbSource" -ForegroundColor Gray
} else {
    Write-Host "  [ERROR] Protobuf files not generated" -ForegroundColor Red
    Write-Host "    Expected location: $buildDir" -ForegroundColor Gray
    exit 1
}

# 4. Build project
Write-Host ""
Write-Host "[4/6] Building project..." -ForegroundColor Yellow
Write-Host "  Command: cmake --build $buildDir --config Release" -ForegroundColor Gray

try {
    cmake --build $buildDir --config Release 2>&1 | Tee-Object -Variable buildOutput
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [ERROR] Build failed" -ForegroundColor Red
        
        # Show error summary
        $errors = $buildOutput | Select-String "error"
        if ($errors) {
            Write-Host ""
            Write-Host "  Error summary:" -ForegroundColor Red
            $errors | Select-Object -First 5 | ForEach-Object { Write-Host "    $_" -ForegroundColor Gray }
        }
        exit 1
    }
    
    Write-Host "  [OK] Build succeeded" -ForegroundColor Green
} catch {
    Write-Host "  [ERROR] Build exception: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# 5. Run unit tests
Write-Host ""
Write-Host "[5/6] Running unit tests..." -ForegroundColor Yellow
Write-Host "  Command: ctest --test-dir $buildDir -C Release -V" -ForegroundColor Gray

try {
    ctest --test-dir $buildDir -C Release -V 2>&1 | Tee-Object -Variable testOutput
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [ERROR] Tests failed" -ForegroundColor Red
        
        # Show failed tests
        $failures = $testOutput | Select-String "Failed"
        if ($failures) {
            Write-Host ""
            Write-Host "  Failed tests:" -ForegroundColor Red
            $failures | ForEach-Object { Write-Host "    $_" -ForegroundColor Gray }
        }
        exit 1
    }
    
    Write-Host "  [OK] All tests passed" -ForegroundColor Green
} catch {
    Write-Host "  [ERROR] Test exception: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# 6. Summary report
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Verification Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "  [OK] vcpkg setup verified" -ForegroundColor Green
Write-Host "  [OK] CMake configuration succeeded" -ForegroundColor Green
Write-Host "  [OK] Protobuf files generated" -ForegroundColor Green
Write-Host "  [OK] Project build succeeded" -ForegroundColor Green
Write-Host "  [OK] All unit tests passed" -ForegroundColor Green
Write-Host ""
Write-Host "SUCCESS: Protobuf integration verified!" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Implement QtNetworkAdapter (Day 3-4)" -ForegroundColor Gray
Write-Host "  2. Implement TCP connection manager (Day 5-8)" -ForegroundColor Gray
Write-Host "  3. Implement text message service (Day 9-12)" -ForegroundColor Gray
Write-Host ""

exit 0
