# Start VS Developer Command Prompt
# Automatically finds and launches Visual Studio 2022 Developer Command Prompt

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Launch VS Developer Command Prompt" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Find VS 2022 installation
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vsWhere)) {
    Write-Host "[ERROR] Visual Studio 2022 not found!" -ForegroundColor Red
    Write-Host "Please install Visual Studio 2022 with C++ Desktop Development workload" -ForegroundColor Yellow
    exit 1
}

# Get VS installation path
$vsPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath

if (-not $vsPath) {
    Write-Host "[ERROR] Visual Studio 2022 C++ tools not found!" -ForegroundColor Red
    Write-Host "Please install C++ Desktop Development workload in VS 2022" -ForegroundColor Yellow
    exit 1
}

# VS Developer Command Prompt script
$devCmdScript = Join-Path $vsPath "Common7\Tools\VsDevCmd.bat"

if (-not (Test-Path $devCmdScript)) {
    Write-Host "[ERROR] VsDevCmd.bat not found at: $devCmdScript" -ForegroundColor Red
    exit 1
}

Write-Host "[OK] Found Visual Studio 2022" -ForegroundColor Green
Write-Host "     Path: $vsPath" -ForegroundColor Gray
Write-Host ""
Write-Host "Launching VS Developer Command Prompt..." -ForegroundColor Yellow
Write-Host ""
Write-Host "In the new CMD window, run:" -ForegroundColor White
Write-Host "  cd e:\Project\FlyKylin" -ForegroundColor Cyan
Write-Host "  .\tools\developer\configure-environment.cmd" -ForegroundColor Cyan
Write-Host "  .\tools\developer\verify-environment.cmd" -ForegroundColor Cyan
Write-Host ""
Write-Host "Note: Use .cmd files (not .ps1) in VS Dev Prompt!" -ForegroundColor Yellow
Write-Host ""

# Launch new command prompt with VS environment
$projectPath = "e:\Project\FlyKylin"
Start-Process cmd.exe -ArgumentList "/k `"$devCmdScript`" && cd /d $projectPath && echo. && echo Ready to build FlyKylin! && echo Run: .\tools\developer\configure-environment.ps1 && echo."

Write-Host "[OK] VS Developer Command Prompt launched!" -ForegroundColor Green
