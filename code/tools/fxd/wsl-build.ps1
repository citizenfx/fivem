<#
.Synopsis
Prepares and runs an incremental FXServer build for WSL.
#>

param(
	[switch] $Rebuild = $false
)

$ErrorActionPreference = 'Stop'

if ($Rebuild) {
	wsl --unregister FXServer-Alpine
}

$distro = (wsl -l -q | Select-String -SimpleMatch "FXServer-Alpine")

if (!$distro) {
	if (!(Test-Path $PSScriptRoot\..\..\build)) {
		New-Item -ItemType Directory $PSScriptRoot\..\..\build
	}

	curl.exe -Lo $env:TEMP\alpine.tar.gz http://dl-cdn.alpinelinux.org/alpine/v3.13/releases/x86_64/alpine-minirootfs-3.13.5-x86_64.tar.gz
	wsl --import FXServer-Alpine $PSScriptRoot\..\..\build\alpine-root $env:TEMP\alpine.tar.gz --version 2

	$execCommands = @(
		"/bin/chmod 755 /",
		"/sbin/apk --no-cache add shadow",
		"/bin/sed -i 's/^export PATH/#export PATH/' /etc/profile"
	)

	foreach ($execCommand in $execCommands) {
		wsl.exe -d FXServer-Alpine sh -c "$execCommand"
	}
}

Push-Location $PSScriptRoot\..\..
wsl -d FXServer-Alpine ./tools/ci/wsl-build.sh (git rev-list -1 HEAD tools/ci/)
Pop-Location
