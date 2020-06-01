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
$IsServer = $false

if ($Game -eq "server") {
    $IsServer = $true
} elseif ($Game -eq "rdr3") {
    $IsRDR = $true
} elseif ($Game -eq "launcher") {
    $IsLauncher = $true
}

if (!$IsServer) {
    ## build UI
    Push-Location $WorkDir
    $UICommit = (git rev-list -1 HEAD ext/ui-build/ ext/cfx-ui/)
    Pop-Location

    Push-Location $WorkDir\ext\ui-build

    if (!(Test-Path data\.commit) -or $UICommit -ne (Get-Content data\.commit)) {
        .\build.cmd | Out-Null
        
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

        if (!(Test-Path $LayoutDir\citizen\re3.rpf)) {
            curl.exe -Lo "$LayoutDir\citizen\re3.rpf" "https://runtime.fivem.net/client/re3.rpf"
        }
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
} else {
    Push-Location $WorkDir\ext\system-resources
    .\build.cmd

    if ($?) {
        New-Item -ItemType Directory -Force $WorkDir\data\server\citizen\system_resources\ | Out-Null
        Copy-Item -Force -Recurse $WorkDir\ext\system-resources\data\* $WorkDir\data\server\citizen\system_resources\
    }

    Pop-Location

    New-Item -ItemType Directory -Force $LayoutDir\citizen | Out-Null

    Copy-Item -Force -Recurse $WorkDir\data\shared\* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\client\v8* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\client\bin\icu* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\server\* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\server_windows\* $LayoutDir\

    Remove-Item -Force $LayoutDir\citizen\.gitignore
    
    # old filename
    if (Test-Path $LayoutDir\citizen\system_resources\monitor\starter.js) {
        Remove-Item -Force $LayoutDir\citizen\system_resources\monitor\starter.js
    }
    
    # useless client-related scripting stuff
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\*.zip
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\*_universal.lua
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\natives_0*.lua
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\natives_2*.lua
    
    Remove-Item -Force $LayoutDir\citizen\scripting\v8\*_universal.d.ts
    Remove-Item -Force $LayoutDir\citizen\scripting\v8\*_universal.js
    Remove-Item -Force $LayoutDir\citizen\scripting\v8\natives_0*.*
    Remove-Item -Force $LayoutDir\citizen\scripting\v8\natives_2*.*
}