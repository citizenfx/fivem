param(
    $Command
)

$ErrorActionPreference = 'Stop'
$pattern = '[\*\?\[\]`$"&|;()@%,#]'

if (!$Command) {
    $Command = 'help'
}

if ($PSScriptRoot -match $pattern) {
    $matches = ([regex]::Matches($PSScriptRoot, $pattern)).Index
    $markerLine = (0..($PSScriptRoot.Length - 1) | ForEach-Object { if ($matches -contains $_) {'^'} else {' '} }) -join ''

    Write-Host "Current working directory contains unsupported characters. Please move or rename the folder:" -ForegroundColor Red
    Write-Host $PSScriptRoot -ForegroundColor Red
    Write-Host $markerLine -ForegroundColor Red
    exit 1
}

$CommandScript = "$PSScriptRoot\code\tools\fxd\$Command.ps1"

if (!(Test-Path $CommandScript)) {
    "fxd: invalid command '$Command' - run 'fxd help' for help"
    return
}

if ($args.Contains('-?') -or $args.Contains('-help') -or $args.Contains('--help') -or $args.Contains('/?')) {
    $helpArgs = ,($args | Select-Object -Skip 1)

    if ($null -eq $helpArgs[0]) {
        $helpArgs = @()
    }

    & "Get-Help" $CommandScript @helpArgs |
        Out-String |
        ForEach-Object { 
            $_ `
            -replace "[^\s]*\\(.*?)\.ps1", "fxd `$1" `
            -replace "Get-Help fxd ([^ ]+)", "fxd `$1 -?"
        }
    return
}

$moreArgs = $args
Invoke-Expression "$CommandScript @moreArgs"