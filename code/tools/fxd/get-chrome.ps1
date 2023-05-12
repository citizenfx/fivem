<#
.Synopsis
Downloads the 64-bit CEF dependency to vendor/cef/.
#>

$WorkDir = "$PSScriptRoot\..\..\..\"
$SaveDir = "$WorkDir\code\build\"

if (!(Test-Path $SaveDir)) { New-Item -ItemType Directory -Force $SaveDir | Out-Null }

$CefName = (Get-Content $PSScriptRoot\..\ci\build.ps1 | Select-String "CefName =") -replace ".*`"(.*?)`"", "`$1"

if (!(Test-Path "$SaveDir\$CefName.zip")) {
	curl.exe --create-dirs -Lo "$SaveDir\$CefName.zip" "https://runtime.fivem.net/build/cef/$CefName.zip"
}

if (Test-Path $WorkDir\vendor\cef) {
	Get-ChildItem -Directory $WorkDir\vendor\cef | Remove-Item -Recurse -Force
}

& $env:WINDIR\system32\tar.exe -C $WorkDir\vendor\cef -xf "$SaveDir\$CefName.zip"
Copy-Item -Force -Recurse $WorkDir\vendor\cef\$CefName\* $WorkDir\vendor\cef\
Remove-Item -Recurse $WorkDir\vendor\cef\$CefName\
