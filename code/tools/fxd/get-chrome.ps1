<#
.Synopsis
Downloads the 64-bit CEF dependency to vendor/cef/.
#>

$WorkDir = "$PSScriptRoot\..\..\..\"
$SaveDir = "$WorkDir\code\build\"

$CefName = (Get-Content $PSScriptRoot\..\ci\build.ps1 | Select-String "CefName =") -replace ".*`"(.*?)`"", "`$1"

if (!(Test-Path "$SaveDir\$CefName.zip")) {
	curl.exe -Lo "$SaveDir\$CefName.zip" "https://runtime.fivem.net/build/cef/$CefName.zip"
}

& $env:WINDIR\system32\tar.exe -C $WorkDir\vendor\cef -xf "$SaveDir\$CefName.zip"
Copy-Item -Force -Recurse $WorkDir\vendor\cef\$CefName\* $WorkDir\vendor\cef\
Remove-Item -Recurse $WorkDir\vendor\cef\$CefName\
