# Install ONNX Runtime C++ API for Windows
# Version: 1.23.2

$ErrorActionPreference = "Stop"

$onnxVersion = "1.23.2"
$onnxUrl = "https://github.com/microsoft/onnxruntime/releases/download/v$onnxVersion/onnxruntime-win-x64-$onnxVersion.zip"
$downloadPath = "$env:TEMP\onnxruntime.zip"
$extractPath = "e:\Project\FlyKylin\3rdparty"
$finalPath = "$extractPath\onnxruntime"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  ONNX Runtime C++ API Installer" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Check if already installed
if (Test-Path "$finalPath\include") {
    Write-Host "[OK] ONNX Runtime already installed" -ForegroundColor Green
    Write-Host "     Path: $finalPath" -ForegroundColor Gray
    exit 0
}

Write-Host "Downloading ONNX Runtime $onnxVersion (about 30MB)..." -ForegroundColor Yellow

try {
    # Download
    $webClient = New-Object System.Net.WebClient
    $webClient.DownloadFile($onnxUrl, $downloadPath)
    Write-Host "[OK] Download completed" -ForegroundColor Green
    
    # Create directory
    if (-not (Test-Path $extractPath)) {
        New-Item -ItemType Directory -Path $extractPath -Force | Out-Null
    }
    
    # Extract
    Write-Host "Extracting..." -ForegroundColor Yellow
    Expand-Archive -Path $downloadPath -DestinationPath $extractPath -Force
    
    # Rename
    $extractedFolder = "$extractPath\onnxruntime-win-x64-$onnxVersion"
    if (Test-Path $extractedFolder) {
        if (Test-Path $finalPath) {
            Remove-Item -Path $finalPath -Recurse -Force
        }
        Rename-Item -Path $extractedFolder -NewName "onnxruntime"
    }
    
    # Cleanup
    Remove-Item -Path $downloadPath -Force
    
    Write-Host "[OK] ONNX Runtime installed successfully!" -ForegroundColor Green
    Write-Host "     Path: $finalPath" -ForegroundColor Gray
    
    # Verify
    if (Test-Path "$finalPath\lib\onnxruntime.dll") {
        $dllSize = (Get-Item "$finalPath\lib\onnxruntime.dll").Length / 1MB
        Write-Host "     DLL size: $([math]::Round($dllSize, 2)) MB" -ForegroundColor Gray
    }
    
} catch {
    Write-Host "[ERROR] Installation failed: $_" -ForegroundColor Red
    Write-Host "Please download manually: $onnxUrl" -ForegroundColor Yellow
    Write-Host "Extract to: $finalPath" -ForegroundColor Yellow
    exit 1
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Installation completed!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
