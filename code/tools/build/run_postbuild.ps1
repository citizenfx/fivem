param (
    [Parameter()]
    [string]
    $LayoutDir,

    [Parameter()]
    [string]
    $Game
)

$LayoutDir = Resolve-Path $LayoutDir

$InstRoot = "$PSScriptRoot\..\..\..\"
$ErrorActionPreference = "Stop"

if ($env:CI) {
    return
}

# Maybe unify with build.ps1?
$WorkDir = "$InstRoot"

$IsRDR = $false
$IsLauncher = $false

if ($Game -eq "rdr3") {
    $IsRDR = $true
} elseif ($Game -eq "launcher") {
    $IsLauncher = $true
}

## build UI
Push-Location $WorkDir
$UICommit = (git rev-list -1 HEAD ext/ui-build/ ext/cfx-ui/)
Pop-Location

Push-Location $WorkDir\ext\ui-build

if (!(Test-Path data\.commit) -or $UICommit -ne (Get-Content data\.commit)) {
    .\build.cmd
    
    $UICommit | Out-File -Encoding ascii -NoNewline data\.commit
}

if ($?) {
    Copy-Item -Force $WorkDir\ext\ui-build\data.zip $LayoutDir\citizen\ui.zip
    Copy-Item -Force $WorkDir\ext\ui-build\data_big.zip $LayoutDir\citizen\ui-big.zip
}

Pop-Location

## setup layout
New-Item -ItemType Directory -Force $LayoutDir\bin

Copy-Item -Force -Recurse $WorkDir\vendor\cef\Release\*.dll $LayoutDir\bin\
Copy-Item -Force -Recurse $WorkDir\vendor\cef\Release\*.bin $LayoutDir\bin\

New-Item -ItemType Directory -Force $LayoutDir\bin\cef

Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\icudtl.dat $LayoutDir\bin\
Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\*.pak $LayoutDir\bin\cef\
Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\locales\en-US.pak $LayoutDir\bin\cef\

if (!$IsLauncher -and !$IsRDR) {
    Copy-Item -Force -Recurse $WorkDir\data\shared\* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\client\* $LayoutDir\
} elseif ($IsLauncher) {
    Copy-Item -Force -Recurse $WorkDir\data\launcher\* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\client\bin\* $LayoutDir\bin\
    Copy-Item -Force -Recurse $WorkDir\data\client\citizen\resources\* $LayoutDir\citizen\resources\
} elseif ($IsRDR) {
    Copy-Item -Force -Recurse $WorkDir\data\shared\* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\client\*.dll $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\client\bin\* $LayoutDir\bin\
    Copy-Item -Force -Recurse $WorkDir\data\client\citizen\clr2 $LayoutDir\citizen\
    Copy-Item -Force -Recurse $WorkDir\data\client\citizen\*.ttf $LayoutDir\citizen\
    Copy-Item -Force -Recurse $WorkDir\data\client\citizen\ros $LayoutDir\citizen\
    Copy-Item -Force -Recurse $WorkDir\data\client\citizen\resources $LayoutDir\citizen\
    Copy-Item -Force -Recurse $WorkDir\data\client_rdr\* $LayoutDir\
}
