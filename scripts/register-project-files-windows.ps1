[CmdletBinding()]
param([string]$Executable = (Join-Path $PSScriptRoot '../build/Release/giada.exe'))
$ErrorActionPreference = 'Stop'
$exe = (Resolve-Path -LiteralPath $Executable).Path
$progId = 'GiadaDevelopment.Project'
$classes = 'HKCU:\Software\Classes'
function Set-DefaultValue([string]$Path, [string]$Value) {
    New-Item -Path $Path -Force | Out-Null
    Set-Item -LiteralPath $Path -Value $Value
}
Set-DefaultValue "$classes\$progId" 'Giada project'
Set-DefaultValue "$classes\$progId\DefaultIcon" ('"' + $exe + '",0')
Set-DefaultValue "$classes\$progId\shell\open\command" ('"' + $exe + '" "%1"')
Set-DefaultValue "$classes\.gptc" $progId
New-Item -Path "$classes\.gptc\OpenWithProgids" -Force | Out-Null
New-ItemProperty -LiteralPath "$classes\.gptc\OpenWithProgids" -Name $progId -PropertyType String -Value '' -Force | Out-Null
# Advertise a named application in Windows' Open With/default-app controls.
New-Item -Path "$classes\Applications\giada.exe\SupportedTypes" -Force | Out-Null
New-ItemProperty -LiteralPath "$classes\Applications\giada.exe" -Name FriendlyAppName -PropertyType String -Value 'Giada Development' -Force | Out-Null
New-ItemProperty -LiteralPath "$classes\Applications\giada.exe\SupportedTypes" -Name '.gptc' -PropertyType String -Value '' -Force | Out-Null
Set-DefaultValue "$classes\Applications\giada.exe\shell\open\command" ('"' + $exe + '" "%1"')
Write-Output 'Registered .gptc project files for Giada Development for the current user.'
