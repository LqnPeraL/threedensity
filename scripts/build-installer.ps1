# Builds Installer/ThreeDensitySetup.exe from the C# sources (WinForms).
param(
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$installerDir = Join-Path $root "Installer"
$outExe = Join-Path $installerDir "ThreeDensitySetup.exe"

$cscCandidates = @(
    "${env:WINDIR}\Microsoft.NET\Framework64\v4.0.30319\csc.exe",
    "${env:WINDIR}\Microsoft.NET\Framework\v4.0.30319\csc.exe"
)
$csc = $cscCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $csc) {
    throw "csc.exe not found. Install .NET Framework 4.x Developer Pack / Windows SDK."
}

$refs = @(
    "System.dll",
    "System.Core.dll",
    "System.Drawing.dll",
    "System.Windows.Forms.dll",
    "System.IO.Compression.dll",
    "System.IO.Compression.FileSystem.dll"
) | ForEach-Object { "/r:$_" }

$sources = @(
    (Join-Path $installerDir "ThreeDensitySetup.cs"),
    (Join-Path $installerDir "LogoData.cs")
)

$icon = Join-Path $installerDir "ThreeDensity.ico"
$iconArg = @()
if (Test-Path $icon) {
    $iconArg = @("/win32icon:$icon")
}

Write-Host "Compiling launcher with $csc"
& $csc /nologo /t:winexe /platform:anycpu /optimize+ `
    /out:$outExe `
    @iconArg `
    @refs `
    @sources

if ($LASTEXITCODE -ne 0 -or -not (Test-Path $outExe)) {
    throw "Launcher build failed"
}

Write-Host "Built $outExe"
Get-Item $outExe | Format-List FullName, Length, LastWriteTime
