<#
.Synopsis
Runs the FxDK dev server for Five.
#>

$ErrorActionPreference = 'Stop'

Push-Location $PSScriptRoot\..\..\..\ext\sdk
cmd.exe /c yarn install
cmd.exe /c yarn start
Pop-Location
