param(
    $Command
)

$ErrorActionPreference = 'Stop'


if (!$Command) {
    $Command = 'help'
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