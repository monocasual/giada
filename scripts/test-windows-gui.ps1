[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Executable,
    [string]$ReportDirectory = (Join-Path $PSScriptRoot '../build/gui-test-results'),
    [ValidateRange(30, 300)][int]$TimeoutSeconds = 120
)
$ErrorActionPreference = 'Stop'
$testExecutable = (Resolve-Path -LiteralPath $Executable).Path
$reports = [IO.Path]::GetFullPath($ReportDirectory)
New-Item -ItemType Directory -Path $reports -Force | Out-Null
$junit = Join-Path $reports 'windows-dialogs.xml'
$stdout = Join-Path $reports 'stdout.log'
$stderr = Join-Path $reports 'stderr.log'
# Only this test process is stopped on timeout. Existing Giada instances remain
# untouched; the fixture uses its own temporary project folders and native dialogs.
$arguments = '--run-tests "[windows-gui]" --reporter junit --out "' + $junit + '"'
$previousArtifacts = $env:GIADA_GUI_TEST_ARTIFACTS
try {
    $env:GIADA_GUI_TEST_ARTIFACTS = Join-Path $reports 'screenshots'
    $testProcess = Start-Process -FilePath $testExecutable -ArgumentList $arguments -PassThru `
        -WindowStyle Hidden -RedirectStandardOutput $stdout -RedirectStandardError $stderr
} finally {
    $env:GIADA_GUI_TEST_ARTIFACTS = $previousArtifacts
}
if (-not $testProcess.WaitForExit($TimeoutSeconds * 1000)) {
    $testProcess.Kill()
    $testProcess.WaitForExit()
    throw "Windows GUI tests timed out. Logs: $reports"
}
if ($testProcess.ExitCode -ne 0) {
    if (Test-Path -LiteralPath $junit) { Get-Content -LiteralPath $junit }
    Get-Content -LiteralPath $stderr
    throw "Windows GUI tests failed with exit code $($testProcess.ExitCode). Logs: $reports"
}
if (-not (Test-Path -LiteralPath $junit)) {
    throw 'The test binary did not produce a GUI test report. Build with WITH_TESTS=ON.'
}
[xml]$results = Get-Content -LiteralPath $junit -Raw
$cases = @($results.SelectNodes('//testcase'))
if ($cases.Count -lt 9) {
    throw "Expected at least nine Windows GUI test cases; found $($cases.Count)."
}
Write-Host "Windows GUI tests passed ($($cases.Count) cases). Report: $junit"
