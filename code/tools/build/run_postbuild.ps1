param (
    [Parameter()]
    [string]
    $LayoutDir,

    [Parameter()]
    [string]
    $Game,

	[Parameter()]
	[string]
	$Configuration
)

$LayoutDir = Resolve-Path $LayoutDir

$InstRoot = "$PSScriptRoot\..\..\..\"
$ErrorActionPreference = "Stop"

if ($env:CI) {
    return
}

try {
	$env:PATH += ";" + (Get-ItemProperty HKLM:\SOFTWARE\WOW6432Node\Yarn).InstallDir + "\bin"
} catch {}

try {
	$env:PATH += ";" + (Get-ItemProperty HKLM:\SOFTWARE\Node.js).InstallPath
} catch {}

$env:PATH += ";C:\Program Files (x86)\Nodist\v-x64\14.17.1"

# Maybe unify with build.ps1?
$WorkDir = "$InstRoot"

$IsDebug = $false

if ($Configuration -eq "Debug") {
	$IsDebug = $true
}

$IsRDR = $false
$IsLauncher = $false
$IsServer = $false
$IsNY = $false

if ($Game -eq "server") {
    $IsServer = $true
} elseif ($Game -eq "rdr3") {
    $IsRDR = $true
} elseif ($Game -eq "ny") {
    $IsNY = $true
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

	## build sdk
    <#Push-Location $WorkDir
    $SDKCommit = (git rev-list -1 HEAD ext/sdk-build/ ext/sdk/)
    Pop-Location

	Push-Location $WorkDir\ext\sdk-build
	if (!(Test-Path sdk-root\.commit) -or $SDKCommit -ne (Get-Content sdk-root\.commit)) {
        .\build.cmd | Out-Null
        
        $SDKCommit | Out-File -Encoding ascii -NoNewline sdk-root\.commit
    }

    if ($?) {
		robocopy $WorkDir\ext\sdk-build\sdk-root\resource\ $LayoutDir\citizen\sdk\sdk-root\ /mir /xo /fft /ndl /njh /njs /nc /ns /np
		xcopy /y /e $WorkDir\ext\sdk\resources\sdk-game\*.* $LayoutDir\citizen\sdk\sdk-game\
    }
    Pop-Location#>

    ## setup layout
    New-Item -ItemType Directory -Force $LayoutDir\bin

    $CefPath = "cef"

    if ($IsNY) {
        $CefPath = "cef32"
    }

    Copy-Item -Force -Recurse $WorkDir\vendor\$CefPath\Release\*.dll $LayoutDir\bin\
    Copy-Item -Force -Recurse $WorkDir\vendor\$CefPath\Release\*.bin $LayoutDir\bin\

    New-Item -ItemType Directory -Force $LayoutDir\bin\cef

    Copy-Item -Force -Recurse $WorkDir\vendor\$CefPath\Resources\icudtl.dat $LayoutDir\bin\
    Copy-Item -Force -Recurse $WorkDir\vendor\$CefPath\Resources\*.pak $LayoutDir\bin\cef\
    Copy-Item -Force -Recurse $WorkDir\vendor\$CefPath\Resources\locales\en-US.pak $LayoutDir\bin\cef\

    if (!$IsLauncher -and !$IsRDR -and !$IsNY) {
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
    } elseif ($IsNY) {
        Copy-Item -Force -Recurse $WorkDir\data\shared\* $LayoutDir\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\clr2 $LayoutDir\citizen\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\*.ttf $LayoutDir\citizen\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\ros $LayoutDir\citizen\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\resources $LayoutDir\citizen\
        Copy-Item -Force -Recurse $WorkDir\data\client_ny\* $LayoutDir\
	}
} else {
    Push-Location $WorkDir\ext\system-resources

    Push-Location $WorkDir
    $SRCommit = (git rev-list -1 HEAD ext/txAdmin ext/system-resources/)
    Pop-Location

    if (!(Test-Path .commit) -or $SRCommit -ne (Get-Content .commit)) {
        .\build.cmd

        $SRCommit | Out-File -Encoding ascii -NoNewline .commit
    }

    if ($?) {
        New-Item -ItemType Directory -Force $WorkDir\data\server\citizen\system_resources\ | Out-Null
        Copy-Item -Force -Recurse $WorkDir\ext\system-resources\data\* $WorkDir\data\server\citizen\system_resources\
    }

    Pop-Location

    New-Item -ItemType Directory -Force $LayoutDir\citizen | Out-Null

    Copy-Item -Force -Recurse $WorkDir\data\shared\* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\server\* $LayoutDir\
    Copy-Item -Force -Recurse $WorkDir\data\server_windows\* $LayoutDir\
    $ConfigurationLower = $Configuration.ToLower()
    Copy-Item -Force -Recurse $WorkDir\data\server_windows_$ConfigurationLower\* $LayoutDir\

    Remove-Item -Force $LayoutDir\citizen\.gitignore
    
    # old filename
    if (Test-Path $LayoutDir\citizen\system_resources\monitor\starter.js) {
        Remove-Item -Force $LayoutDir\citizen\system_resources\monitor\starter.js
    }

	# for public source builds, remove adhesive from the components list
	if (!(Test-Path $LayoutDir\svadhesive.dll)) {
		(Get-Content $LayoutDir\components.json | ConvertFrom-Json) | 
		Where-Object { $_ -ne "svadhesive" } | 
		ConvertTo-Json | 
		Set-Content $LayoutDir\components.json
	}
    
    # useless client-related scripting stuff
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\natives_0*.zip
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\natives_2*.zip
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\*_universal.lua
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\*_universal.zip
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\natives_0*.lua
    Remove-Item -Force $LayoutDir\citizen\scripting\lua\natives_2*.lua
    
    Remove-Item -Force $LayoutDir\citizen\scripting\v8\*_universal.d.ts
    Remove-Item -Force $LayoutDir\citizen\scripting\v8\*_universal.js
    Remove-Item -Force $LayoutDir\citizen\scripting\v8\natives_0*.*
    Remove-Item -Force $LayoutDir\citizen\scripting\v8\natives_2*.*
}
