$InstRoot = "$PSScriptRoot\..\..\..\"
$ErrorActionPreference = "Stop"

if ($env:CI) {
	Set-Content -Path "$PSScriptRoot\prebuild_run.txt" -Value ($null)
	(Get-Item "$PSScriptRoot\prebuild_run.txt").LastWriteTime = Get-Date

    return 0
}

try {
	if (!([string](py -3 --version 2>&1)).StartsWith("Python 3")) {
		"PREBUILD : error PY3X : ``py`` in PATH is not Python 3." | Write-Host
		return 1;
	}
} catch {
	"PREBUILD : error PY30 : Could not execute ``py --version``. Make sure py.exe is installed." | Write-Host
	return 1;
}

try {
	$env:PATH += ";" + (Get-ItemProperty HKLM:\SOFTWARE\WOW6432Node\Yarn).InstallDir + "\bin"
} catch {}

try {
	$env:PATH += ";" + (Get-ItemProperty HKLM:\SOFTWARE\Node.js).InstallPath
} catch {}

$env:PATH += ";C:\Program Files (x86)\Nodist\v-x64\14.17.1"

Push-Location $InstRoot

if (!(Test-Path $InstRoot\code\client\clrcore\NativesFive.cs)) {
	try {
		yarn --version 2>&1 | Out-Null
	} catch {
		"PREBUILD : error YARN : Could not execute ``yarn``, is it in your PATH?" | Write-Host
		return 1;
	}

	try {
		node --version 2>&1 | Out-Null
	} catch {
		"PREBUILD : error NODE : Could not execute ``node``, is it in your PATH?" | Write-Host
		return 1;
	}

    Invoke-Expression "$InstRoot\prebuild_natives.cmd"

	if (!$?) {
		throw "natives failed"
	}
}

if (!(Test-Path $InstRoot\vendor\udis86\libudis86\itab.c)) {
    Invoke-Expression "$InstRoot\code\prebuild_udis86.cmd"

	if (!$?) {
		throw "udis failed"
	}
}

if (!(Test-Path $InstRoot\code\tools\idl\deps) -or ((Get-ChildItem $InstRoot\code\tools\idl\deps).Length -eq 0)) {
    Invoke-Expression "$InstRoot\code\prebuild_misc.cmd"

	if (!$?) {
		throw "misc failed"
	}
}

Pop-Location

if (!(Test-Path $InstRoot\code\client\clrcore\NativesFive.cs)) {
	throw "natives failed 2"
}

if (!(Test-Path $InstRoot\vendor\udis86\libudis86\itab.c)) {
	throw "udis failed 2"
}

if (!(Test-Path $InstRoot\code\tools\idl\deps) -or ((Get-ChildItem $InstRoot\code\tools\idl\deps).Length -eq 0)) {
	throw "misc failed 2"
}

Set-Content -Path "$PSScriptRoot\prebuild_run.txt" -Value ($null)
(Get-Item "$PSScriptRoot\prebuild_run.txt").LastWriteTime = Get-Date
