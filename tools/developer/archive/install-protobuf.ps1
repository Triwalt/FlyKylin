# Install Protobuf Compiler
# Uses Chocolatey package manager

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Protobuf Compiler Installer" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Check if already installed
try {
    $protocVersion = & protoc --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[OK] Protobuf already installed: $protocVersion" -ForegroundColor Green
        exit 0
    }
} catch {
    # Not installed, continue
}

Write-Host "Protobuf not found, attempting installation..." -ForegroundColor Yellow

# Check Chocolatey
try {
    $null = & choco --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Using Chocolatey for installation..." -ForegroundColor Gray
        Write-Host "Note: Requires administrator privileges" -ForegroundColor Yellow
        
        # Install
        try {
            & choco install protoc -y
            
            # Verify
            $protocVersion = & protoc --version 2>&1
            if ($LASTEXITCODE -eq 0) {
                Write-Host "[OK] Protobuf installed successfully!" -ForegroundColor Green
                Write-Host "     Version: $protocVersion" -ForegroundColor Gray
                exit 0
            } else {
                throw "protoc not found after installation"
            }
        } catch {
            Write-Host "[ERROR] Chocolatey installation failed: $_" -ForegroundColor Red
        }
    }
} catch {
    Write-Host "[INFO] Chocolatey not found" -ForegroundColor Gray
}

# Alternative installation methods
Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "  Alternative Installation Methods" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""
Write-Host "1. Install Chocolatey first, then run this script again:" -ForegroundColor White
Write-Host "   Set-ExecutionPolicy Bypass -Scope Process -Force" -ForegroundColor Gray
Write-Host "   iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" -ForegroundColor Gray
Write-Host ""
Write-Host "2. Use vcpkg:" -ForegroundColor White
Write-Host "   .\vcpkg\vcpkg install protobuf:x64-windows" -ForegroundColor Gray
Write-Host ""
Write-Host "3. Manual download:" -ForegroundColor White
Write-Host "   https://github.com/protocolbuffers/protobuf/releases" -ForegroundColor Gray
Write-Host "   Extract and add to PATH" -ForegroundColor Gray
Write-Host ""

exit 1
