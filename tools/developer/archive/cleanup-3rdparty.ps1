#!/usr/bin/env pwsh
# ============================================
# Third-Party Dependencies Cleanup Script
# ============================================
# Purpose: Clean up large third-party files that should not be committed
# Usage: .\tools\developer\cleanup-3rdparty.ps1 [-WhatIf]

param(
    [switch]$WhatIf
)

$ProjectRoot = "e:\Project\FlyKylin"
Set-Location $ProjectRoot

Write-Host @"
╔════════════════════════════════════════════════╗
║     Third-Party Dependencies Cleanup           ║
╚════════════════════════════════════════════════╝
"@ -ForegroundColor Cyan

# Directories to clean
$dirsToClean = @(
    @{Path="3rdparty/rknn-toolkit2"; Size="~2GB"; Desc="RKNN Toolkit 2 (RK3566 NPU SDK)"},
    @{Path="3rdparty/onnxruntime"; Size="~500MB"; Desc="ONNX Runtime libraries"},
    @{Path="3rdparty/rknn_api"; Size="~100MB"; Desc="RKNN Runtime API"}
)

# Check what will be removed
Write-Host "`nScanning for large third-party files...`n" -ForegroundColor Yellow

$totalSize = 0
$foundItems = @()

foreach ($item in $dirsToClean) {
    $path = $item.Path
    
    if (Test-Path $path) {
        $size = (Get-ChildItem $path -Recurse -File -ErrorAction SilentlyContinue | 
                 Measure-Object -Property Length -Sum).Sum
        $sizeMB = [math]::Round($size / 1MB, 2)
        $totalSize += $size
        
        Write-Host "  Found: $path" -ForegroundColor Green
        Write-Host "    Size: $sizeMB MB" -ForegroundColor Gray
        Write-Host "    Desc: $($item.Desc)" -ForegroundColor Gray
        Write-Host ""
        
        $foundItems += @{Path=$path; Size=$sizeMB; Desc=$item.Desc}
    } else {
        Write-Host "  Skip: $path (not found)" -ForegroundColor DarkGray
    }
}

if ($foundItems.Count -eq 0) {
    Write-Host "✓ No large third-party files found. Already clean!" -ForegroundColor Green
    exit 0
}

$totalSizeMB = [math]::Round($totalSize / 1MB, 2)
Write-Host "Total size to be removed: $totalSizeMB MB`n" -ForegroundColor Yellow

# Confirm deletion
if (!$WhatIf) {
    Write-Host "⚠️  WARNING: This will permanently delete the above directories!" -ForegroundColor Red
    Write-Host "   (They are already excluded from Git via .gitignore)" -ForegroundColor Gray
    Write-Host ""
    $confirm = Read-Host "Continue with deletion? (y/N)"
    
    if ($confirm -ne "y") {
        Write-Host "`n✗ Operation cancelled" -ForegroundColor Yellow
        exit 0
    }
}

# Perform cleanup
Write-Host "`nCleaning up...`n" -ForegroundColor Yellow

foreach ($item in $foundItems) {
    $path = $item.Path
    
    if ($WhatIf) {
        Write-Host "  [WHATIF] Would remove: $path ($($item.Size) MB)" -ForegroundColor Cyan
    } else {
        try {
            Remove-Item -Path $path -Recurse -Force -ErrorAction Stop
            Write-Host "  ✓ Removed: $path ($($item.Size) MB)" -ForegroundColor Green
        } catch {
            Write-Host "  ✗ Failed to remove: $path" -ForegroundColor Red
            Write-Host "    Error: $_" -ForegroundColor Gray
        }
    }
}

# Summary
Write-Host @"

╔════════════════════════════════════════════════╗
║              Cleanup Summary                   ║
╚════════════════════════════════════════════════╝

"@ -ForegroundColor Green

if ($WhatIf) {
    Write-Host "This was a simulation (--WhatIf mode)" -ForegroundColor Yellow
    Write-Host "No files were actually deleted.`n" -ForegroundColor Yellow
    Write-Host "Run without --WhatIf to perform actual cleanup:`n" -ForegroundColor Gray
    Write-Host "  .\tools\developer\cleanup-3rdparty.ps1`n" -ForegroundColor Cyan
} else {
    Write-Host "✓ Cleanup completed!" -ForegroundColor Green
    Write-Host "  Freed space: $totalSizeMB MB`n" -ForegroundColor Gray
    
    Write-Host "To reinstall dependencies:" -ForegroundColor Yellow
    Write-Host "  .\tools\developer\setup\install-dependencies.cmd`n" -ForegroundColor Cyan
    
    Write-Host "Or download manually:" -ForegroundColor Yellow
    Write-Host "  RKNN: https://github.com/airockchip/rknn-toolkit2" -ForegroundColor Cyan
    Write-Host "  ONNX: https://github.com/microsoft/onnxruntime/releases`n" -ForegroundColor Cyan
}

Write-Host "See 3rdparty/README.md for detailed instructions.`n" -ForegroundColor Gray
