<#
.Synopsis
Runs MSBuild on the core native project.
#>

[CmdletBinding(PositionalBinding=$false)]
param (
	[string] $Configuration = 'Debug',
	[string] $Game = 'five'
)

Import-Module $PSScriptRoot\.helpers.psm1

Invoke-VSInit

$BuildPath = "$PSScriptRoot\..\..\build\$Game"

if ($Game -eq "server") {
	$BuildPath += "\windows"
}

if (!(Test-Path $BuildPath\CitizenMP.sln)) {
	.\gen.ps1 -Game $Game
}

Remove-Item env:\platform

Invoke-Expression "& $PSScriptRoot\..\ci\nuget.exe restore $BuildPath\CitizenMP.sln"

$env:UseMultiToolTask = "true"
$env:EnforceProcessCountAcrossBuilds = "true"
msbuild /p:configuration=$Configuration /m @args $BuildPath\CitizenMP.sln
