[CmdletBinding()]
param(
    [string]$CMake = 'cmake',
    [string]$VcpkgRoot = $env:VCPKG_ROOT,
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo')]
    [string]$Configuration = 'Release',
    [ValidateRange(1, 64)]
    [int]$Jobs = 4,
    [string]$ToolsetDirectory,
    [string]$OutputDirectory,
    [switch]$SkipConfigure,
    [switch]$SkipGuiTests,
    [switch]$Launch
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $repoRoot 'build'

function Invoke-Checked {
    param([string]$Program, [string[]]$Arguments)
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Program failed with exit code $LASTEXITCODE"
    }
}

if (-not $IsWindows -and $PSVersionTable.PSEdition -eq 'Core') {
    throw 'This helper builds Giada on Windows.'
}
if (-not (Get-Command $CMake -ErrorAction SilentlyContinue)) {
    throw 'CMake 3.29 or newer is required. Pass its executable using -CMake.'
}
if (-not $SkipConfigure) {
    if (-not $VcpkgRoot -or -not (Test-Path -LiteralPath (Join-Path $VcpkgRoot 'scripts/buildsystems/vcpkg.cmake'))) {
        throw 'Pass a bootstrapped vcpkg checkout using -VcpkgRoot, or set VCPKG_ROOT.'
    }
    Invoke-Checked git @('-C', $repoRoot, 'submodule', 'update', '--init', '--recursive')
    Invoke-Checked $CMake @('-S', $repoRoot, '-B', $buildDirectory,
        '-G', 'Visual Studio 17 2022', '-A', 'x64',
        "-DCMAKE_TOOLCHAIN_FILE=$VcpkgRoot/scripts/buildsystems/vcpkg.cmake",
        '-DVCPKG_TARGET_TRIPLET=x64-windows', '-DWITH_VST3=ON', '-DWITH_TESTS=ON',
        '-DCMAKE_CXX_SCAN_FOR_MODULES=OFF')
}

$buildArguments = @('--build', $buildDirectory, '--config', $Configuration,
    '--parallel', "$Jobs")
if ($ToolsetDirectory) {
    $toolset = [IO.Path]::GetFullPath($ToolsetDirectory).TrimEnd('\', '/').Replace('\', '/') + '/'
    if (-not (Test-Path -LiteralPath ($toolset + 'bin/Hostx64/x64/cl.exe'))) {
        throw 'ToolsetDirectory must contain the MSVC include, lib, and bin directories.'
    }
    $buildArguments += @('--', "/p:VCToolsInstallDir=$toolset",
        "/p:CLToolPath=${toolset}bin/Hostx64/x64/",
        "/p:LinkToolPath=${toolset}bin/Hostx64/x64/",
        '/p:UseMultiToolTask=true', '/p:EnforceProcessCountAcrossBuilds=true',
        "/p:CL_MPCount=$Jobs")
}
Invoke-Checked $CMake $buildArguments
$binaryDirectory = Join-Path $buildDirectory $Configuration
$executable = Join-Path $binaryDirectory 'giada.exe'
Invoke-Checked $executable @('--run-tests')
if (-not $SkipGuiTests) {
    & (Join-Path $PSScriptRoot 'test-windows-gui.ps1') -Executable $executable
}

if ($OutputDirectory) {
    $distribution = [IO.Path]::GetFullPath($OutputDirectory)
    New-Item -ItemType Directory -Path $distribution -Force | Out-Null
    Copy-Item -LiteralPath $executable -Destination $distribution -Force
    Get-ChildItem -LiteralPath $binaryDirectory -Filter '*.dll' -File |
        Copy-Item -Destination $distribution -Force
    $dependencyBin = if ($Configuration -eq 'Debug') {
        Join-Path $buildDirectory 'vcpkg_installed/x64-windows/debug/bin'
    } else {
        Join-Path $buildDirectory 'vcpkg_installed/x64-windows/bin'
    }
    # Include transitive codec DLLs as well as directly linked dependencies.
    # The upstream 1.6.0 archive omitted libmp3lame.dll.
    Get-ChildItem -LiteralPath $dependencyBin -Filter '*.dll' -File |
        Copy-Item -Destination $distribution -Force
    Copy-Item -LiteralPath (Join-Path $repoRoot 'COPYING') -Destination $distribution -Force
    $executable = Join-Path $distribution 'giada.exe'
}

Write-Host "Built and tested: $executable"
if ($Launch) {
    Start-Process -FilePath $executable -WorkingDirectory (Split-Path -Parent $executable)
}
