# FlyKylin Windows Build Script

param(
    [Parameter(Position=0)]
    [ValidateSet("configure", "build", "clean", "package", "all")]
    [string]$Action = "build",
    
    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$Config = "Release",
    
    [switch]$DetailedOutput,
    [switch]$Help
)

# Configuration
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
if (-not $ProjectRoot -or -not (Test-Path (Join-Path $ProjectRoot "CMakeLists.txt"))) {
    $ProjectRoot = (Get-Location).Path
}
$BuildDir = Join-Path $ProjectRoot "build\windows-x64-$($Config.ToLower())"
$BinDir = Join-Path $BuildDir "bin\$Config"

# Output functions
function Write-Info { Write-Host "[INFO] $args" -ForegroundColor Green }
function Write-Warn { Write-Host "[WARN] $args" -ForegroundColor Yellow }
function Write-Err { Write-Host "[ERROR] $args" -ForegroundColor Red }
function Write-Step { Write-Host "[STEP] $args" -ForegroundColor Cyan }

# Show help
function Show-Help {
    Write-Host @"
FlyKylin Windows Build Script

Usage: .\build-windows.ps1 [Action] [-Config <config>] [-Verbose] [-Help]

Action:
  configure   Configure CMake project
  build       Build project (default)
  clean       Clean build directory
  package     Package application
  all         Configure + Build + Package

Parameters:
  -Config     Build config: Debug, Release (default), RelWithDebInfo
  -DetailedOutput    Show detailed output
  -Help       Show help

Examples:
  .\build-windows.ps1 build
  .\build-windows.ps1 all -Config Release
  .\build-windows.ps1 configure -DetailedOutput
"@
}

# Check environment
function Test-Environment {
    Write-Step "Checking build environment..."
    
    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        Write-Err "CMake not installed or not in PATH"
        return $false
    }
    
    if (-not $env:Qt6_DIR) {
        Write-Warn "Qt6_DIR environment variable not set"
        $QtPaths = @(
            "C:\Qt\6.5.3\msvc2019_64",
            "C:\Qt\6.6.0\msvc2019_64",
            "C:\Qt\6.7.0\msvc2019_64"
        )
        foreach ($path in $QtPaths) {
            if (Test-Path $path) {
                $env:Qt6_DIR = $path
                Write-Info "Auto-detected Qt: $path"
                break
            }
        }
    }
    
    if (-not $env:VCPKG_ROOT) {
        if (Test-Path "C:\vcpkg") {
            $env:VCPKG_ROOT = "C:\vcpkg"
            Write-Info "Auto-detected vcpkg: C:\vcpkg"
        } else {
            Write-Warn "VCPKG_ROOT environment variable not set"
        }
    }
    
    Write-Info "Environment check completed"
    return $true
}

# Configure project
function Invoke-Configure {
    Write-Step "Configuring CMake project..."
    
    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
    }
    
    $cmakeArgs = @(
        "-S", $ProjectRoot,
        "-B", $BuildDir,
        "-G", "Visual Studio 17 2022",
        "-A", "x64"
    )
    
    if ($env:VCPKG_ROOT) {
        $toolchainFile = Join-Path $env:VCPKG_ROOT "scripts\buildsystems\vcpkg.cmake"
        if (Test-Path $toolchainFile) {
            $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$toolchainFile"
        }
    }
    
    if ($DetailedOutput) {
        $cmakeArgs += "--log-level=VERBOSE"
    }
    
    Write-Info "CMake args: $($cmakeArgs -join ' ')"
    
    & cmake $cmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-Err "CMake configuration failed"
        return $false
    }
    
    Write-Info "Configuration completed"
    return $true
}

# Build project
function Invoke-Build {
    Write-Step "Building project..."
    
    if (-not (Test-Path (Join-Path $BuildDir "CMakeCache.txt"))) {
        Write-Warn "Project not configured, running configure first..."
        if (-not (Invoke-Configure)) {
            return $false
        }
    }
    
    $buildArgs = @(
        "--build", $BuildDir,
        "--config", $Config,
        "--parallel"
    )
    
    if ($DetailedOutput) {
        $buildArgs += "--verbose"
    }
    
    & cmake $buildArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-Err "Build failed"
        return $false
    }
    
    Write-Info "Build completed: $BinDir"
    return $true
}

# Clean build
function Invoke-Clean {
    Write-Step "Cleaning build directory..."
    
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Info "Deleted: $BuildDir"
    } else {
        Write-Info "Build directory does not exist"
    }
    
    return $true
}

# Package application
function Invoke-Package {
    Write-Step "Packaging application..."
    
    $exePath = Join-Path $BinDir "FlyKylin.exe"
    
    if (-not (Test-Path $exePath)) {
        Write-Err "Executable not found: $exePath"
        Write-Err "Please build the project first"
        return $false
    }
    
    $windeployqt = $null
    if ($env:Qt6_DIR) {
        $windeployqt = Join-Path $env:Qt6_DIR "bin\windeployqt.exe"
    }
    
    if ($windeployqt -and (Test-Path $windeployqt)) {
        Write-Info "Running windeployqt..."
        
        $qmlDir = Join-Path $ProjectRoot "src\ui\qml"
        
        & $windeployqt --qmldir $qmlDir $exePath
        
        if ($LASTEXITCODE -ne 0) {
            Write-Warn "windeployqt returned non-zero exit code"
        }
    } else {
        Write-Warn "windeployqt not found, skipping Qt dependency deployment"
    }
    
    $onnxDll = Join-Path $ProjectRoot "3rdparty\onnxruntime\lib\onnxruntime.dll"
    if (Test-Path $onnxDll) {
        Copy-Item $onnxDll $BinDir -Force
        Write-Info "Copied ONNX Runtime DLL"
    }
    
    Write-Info "Packaging completed: $BinDir"
    return $true
}

# Main function
function Main {
    if ($Help) {
        Show-Help
        return
    }
    
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host " FlyKylin Windows Build" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Project root: $ProjectRoot"
    Write-Host "Build dir: $BuildDir"
    Write-Host "Build config: $Config"
    Write-Host ""
    
    if (-not (Test-Environment)) {
        exit 1
    }
    
    $success = $true
    
    switch ($Action) {
        "configure" {
            $success = Invoke-Configure
        }
        "build" {
            $success = Invoke-Build
        }
        "clean" {
            $success = Invoke-Clean
        }
        "package" {
            $success = Invoke-Package
        }
        "all" {
            $success = Invoke-Configure
            if ($success) { $success = Invoke-Build }
            if ($success) { $success = Invoke-Package }
        }
    }
    
    Write-Host ""
    if ($success) {
        Write-Info "Operation completed!"
    } else {
        Write-Err "Operation failed!"
        exit 1
    }
}

Main
