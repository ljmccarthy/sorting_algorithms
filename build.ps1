param(
    [ValidateSet("Debug","Release")]
    [string]$Configuration="Release",
    [string]$Platform="x64"
)

$ErrorActionPreference = "Stop"

function Usage {
    Write-Host "Usage: .\build.ps1 Debug|Release"
    exit 1
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    Write-Host "cl.exe not found in PATH. Run this from a 'Developer Command Prompt for VS' or run vcvarsall.bat first."
    exit 2
}

if ($Configuration -eq "Debug") {
    $ConfigurationFlags = "/MTd /Od /Zi"
} else {
    $ConfigurationFlags = "/MT /O2 /DNDEBUG"
}

$BuildDir = Join-Path $PSScriptRoot "build" $Platform $Configuration
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

$SrcFiles = Get-ChildItem -Path .\src -Filter *.c -File | ForEach-Object { $_.FullName }
$SrcFiles += Get-ChildItem -Path .\third_party -Filter *.c -File | ForEach-Object { $_.FullName }
$OutputExe = Join-Path $BuildDir "test_sort.exe"
$clArgs = "/nologo /MP /W4 /std:c17 $ConfigurationFlags $($SrcFiles -join "") /Fe:`"$OutputExe`""

& cl.exe $clArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed (exit code $LASTEXITCODE)."
    exit $LASTEXITCODE
}
