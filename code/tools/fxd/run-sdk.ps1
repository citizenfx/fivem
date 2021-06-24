<#
.Synopsis
Starts Five, listening to the FxDK dev server.
#>

Push-Location $PSScriptRoot\..\..\bin\five\debug
$SdkDir = (Resolve-Path $PSScriptRoot\..\..\..\ext\sdk\resources\sdk-root\).Path.TrimEnd("\").Replace("\", "/")
$LowercaseComputerName = $env:COMPUTERNAME.ToLowerInvariant()
.\FiveM.exe -fxdk +set sdk_url "http://${LowercaseComputerName}:3000/" +set sdk_root_path "$SdkDir"
