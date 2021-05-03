<#
.Synopsis
Opens Visual Studio for the native project.
#>

[CmdletBinding(PositionalBinding=$false)]
param (
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

Invoke-Expression "& $PSScriptRoot\..\ci\nuget.exe restore $BuildPath\CitizenMP.sln"

$CurBranch = (git branch --show-current) -replace '[-/\\]', '_'

Copy-Item -Force $BuildPath\CitizenMP.sln $BuildPath\CitizenMP_${Game}_${CurBranch}.sln
devenv.exe "$BuildPath\CitizenMP_${Game}_${CurBranch}.sln"
