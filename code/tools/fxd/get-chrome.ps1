<#
.Synopsis
Downloads the 64-bit CEF dependency to vendor/cef/.
#>

$WorkDir = "$PSScriptRoot\..\..\..\"
$SaveDir = "$WorkDir\code\build\"

if (!(Test-Path $SaveDir)) { New-Item -ItemType Directory -Force $SaveDir | Out-Null }

$CefName = (Get-Content -Encoding ascii $WorkDir\vendor\cef\cef_build_name.txt).Trim()

if (!(Test-Path "$SaveDir\$CefName.tar.bz2")) {
	echo "Downloading CEF $CefName..."
	curl.exe --create-dirs -Lo "$SaveDir\$CefName.tar.bz2" "https://cef-builds.spotifycdn.com/$CefName.tar.bz2"
} else {
	echo "CEF $CefName already downloaded at $SaveDir\$CefName.tar.bz2."
}

if (Test-Path $WorkDir\vendor\cef) {
	Get-ChildItem -Directory $WorkDir\vendor\cef | Remove-Item -Recurse -Force
}

& $env:WINDIR\system32\tar.exe -C $WorkDir\vendor\cef -xf "$SaveDir\$CefName.tar.bz2"
echo "Placing $WorkDir\vendor\cef\"
Copy-Item -Force -Recurse $WorkDir\vendor\cef\$CefName\* $WorkDir\vendor\cef\
Remove-Item -Recurse $WorkDir\vendor\cef\$CefName\
