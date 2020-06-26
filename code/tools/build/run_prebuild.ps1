$InstRoot = "$PSScriptRoot\..\..\..\"

if (([string](python --version 2>&1)).StartsWith("Python 3")) {
    "PREBUILD : error PY27 : `python` in PATH is Python 3, not Python 2.
Please set PATH for MSBuild/VS to contain Python 2.7 before Python 3.x." | Write-Host
    return 1;
}

$ErrorActionPreference = "Stop"

Push-Location $InstRoot

if (!(Test-Path $InstRoot\code\client\clrcore\NativesFive.cs)) {
    Invoke-Expression "$InstRoot\prebuild_natives.cmd"
}

if (!(Test-Path $InstRoot\vendor\udis86\libudis86\itab.c)) {
    Invoke-Expression "$InstRoot\code\prebuild_udis86.cmd"
}

if (!(Test-Path $InstRoot\code\tools\idl\deps)) {
    Invoke-Expression "$InstRoot\code\prebuild_misc.cmd"
}

Pop-Location