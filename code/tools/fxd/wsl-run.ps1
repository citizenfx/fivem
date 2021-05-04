<#
.Synopsis
Runs the built WSL server install in the specified data directory.
#>

param(
	[Parameter(Mandatory = $true, Position = 0)]
	[string] $Path,

	[switch] $WithDebugger
)

Push-Location $Path

wsl -d FXServer-Alpine cp -a /lib/ld-musl-x86_64.so.1 /opt/cfx-server/

if (!$WithDebugger) {
	wsl -d FXServer-Alpine /opt/cfx-server/ld-musl-x86_64.so.1 /opt/cfx-server/FXServer +set citizen_dir /opt/cfx-server/citizen/ +exec server.cfg @args
} else {
	# TODO: gdbinit
	wsl -d FXServer-Alpine apk add gdb
	wsl -d FXServer-Alpine gdb --args /opt/cfx-server/FXServer +set citizen_dir /opt/cfx-server/citizen/ +exec server.cfg @args
}
Pop-Location
