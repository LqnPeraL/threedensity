# Zip a local Unreal Win64 package for GitHub Releases.
# 1) Package the project in the Editor (or RunUAT BuildCookRun) to e.g. Packaged/Windows
# 2) Run:  pwsh ./scripts/package-win64.ps1
# 3) Commit release-assets/ThreeDensity-Win64.zip (optional) or upload onto the CI release
param(
    [string]$PackageDir = "",
    [string]$OutZip = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot

if (-not $PackageDir) {
    $candidates = @(
        (Join-Path $root "Packaged\Windows"),
        (Join-Path $root "Packaged\WindowsNoEditor"),
        (Join-Path $root "Saved\StagedBuilds\Windows")
    )
    $PackageDir = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if (-not $PackageDir -or -not (Test-Path $PackageDir)) {
    throw "No packaged Windows build found. Package the game first, then pass -PackageDir."
}

$assets = Join-Path $root "release-assets"
New-Item -ItemType Directory -Force -Path $assets | Out-Null
if (-not $OutZip) {
    $OutZip = Join-Path $assets "ThreeDensity-Win64.zip"
}

if (Test-Path $OutZip) { Remove-Item $OutZip -Force }

Write-Host "Zipping $PackageDir -> $OutZip"
Compress-Archive -Path (Join-Path $PackageDir "*") -DestinationPath $OutZip -CompressionLevel Optimal
Get-Item $OutZip | Format-List FullName, Length, LastWriteTime
Write-Host "Done. Push this zip (or upload it to the latest GitHub Release) to test your cooked build."
