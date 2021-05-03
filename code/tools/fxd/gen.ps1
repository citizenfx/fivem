<#
.Synopsis
Generates Visual Studio project files for the specified Cfx project.

.Description
This command can be used to (re)generate Visual Studio project files for a game specified on the command line.

By specifying the -Game argument, a different game can be selected.

.Parameter Game
Specifies the game to build project files for. This should be any of 'five', 'ny', 'server' or 'rdr3'.

.Example
fxd gen

.Example
fxd gen -game server
#>
[CmdletBinding(PositionalBinding=$false)]
param(
	[string]$Game = 'five'
)

Push-Location "$PSScriptRoot\..\..\"
& "$PSScriptRoot\..\ci\premake5.exe" vs2019 "--game=$Game"
Pop-Location
