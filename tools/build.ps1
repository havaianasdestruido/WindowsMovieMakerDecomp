# build.ps1 - Build automation for WMMR
# Usage:
#   .\tools\build.ps1                    # Full Debug build
#   .\tools\build.ps1 -Config Release    # Release build
#   .\tools\build.ps1 -Clean             # Clean build
#   .\tools\build.ps1 -Target MovieMakerCore  # Single target
#   .\tools\build.ps1 -Analyze           # Build + analyze binaries

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",

    [string]$Target = "",

    [switch]$Clean,

    [switch]$Analyze,

    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$BuildDir = Join-Path $ProjectRoot "build"
$CMakeExe = "C:\Program Files\CMake\bin\cmake.exe"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  WMMR Build System" -ForegroundColor Cyan
Write-Host "  Config: $Config" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Configure if needed
if ($Clean -or !(Test-Path "$BuildDir\CMakeCache.txt")) {
    Write-Host "[1/3] Configuring CMake..." -ForegroundColor Yellow
    if ($Clean -and (Test-Path $BuildDir)) {
        Remove-Item -Recurse -Force $BuildDir
    }
    & $CMakeExe -S $ProjectRoot -B $BuildDir -DCMAKE_BUILD_TYPE=$Config
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  CMake configure FAILED" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "[1/3] CMake cache found, skipping configure" -ForegroundColor DarkGray
}

# Step 2: Build
$targetArgs = @()
if ($Target) {
    $targetArgs = @("--target", $Target)
    Write-Host "[2/3] Building target: $Target ($Config)..." -ForegroundColor Yellow
} else {
    Write-Host "[2/3] Building all targets ($Config)..." -ForegroundColor Yellow
}

$buildArgs = @("--build", $BuildDir, "--config", $Config) + $targetArgs
if ($Verbose) {
    $buildArgs += "--verbose"
}

& $CMakeExe @buildArgs 2>&1 | ForEach-Object {
    if ($_ -match "error C\d+:") {
        Write-Host "  $_" -ForegroundColor Red
    } elseif ($_ -match "error LNK") {
        Write-Host "  $_" -ForegroundColor Red
    } elseif ($_ -match "warning C\d+:") {
        if ($Verbose) { Write-Host "  $_" -ForegroundColor DarkYellow }
    } else {
        if ($Verbose) { Write-Host "  $_" -ForegroundColor DarkGray }
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "  Build FAILED (exit code $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "  Build succeeded!" -ForegroundColor Green

# Step 3: Analyze
if ($Analyze) {
    Write-Host ""
    Write-Host "[3/3] Analyzing binaries..." -ForegroundColor Yellow

    $binDir = Join-Path $BuildDir "bin\$Config"
    $peAnalyzer = Join-Path $PSScriptRoot "pe_analyzer.py"
    $rttiExtractor = Join-Path $PSScriptRoot "rtti_extractor.py"

    if (Test-Path $peAnalyzer) {
        Get-ChildItem "$binDir\*.exe", "$binDir\*.dll" -ErrorAction SilentlyContinue | ForEach-Object {
            Write-Host "  Analyzing $($_.Name)..." -ForegroundColor DarkGray
            python $peAnalyzer $_.FullName 2>&1 | Write-Host -ForegroundColor DarkGray
        }
    }
}

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
$binDir = Join-Path $BuildDir "bin\$Config"
$exes = Get-ChildItem "$binDir\*.exe" -ErrorAction SilentlyContinue
$dlls = Get-ChildItem "$binDir\*.dll" -ErrorAction SilentlyContinue
Write-Host "  Output: $binDir" -ForegroundColor Cyan
Write-Host "  EXEs: $($exes.Count) | DLLs: $($dlls.Count)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
