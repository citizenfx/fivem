@echo off

setlocal EnableDelayedExpansion

set path=C:\msys64\usr\bin;%path%

pacman --noconfirm --needed -Sy make curl diffutils libcurl

pushd ext\natives\
mkdir in
curl -z in\natives_global.lua -Lo in\natives_global_new.lua http://runtime.fivem.net/doc/natives.lua
curl -z in\natives_cfx.lua -Lo in\natives_cfx_new.lua http://runtime.fivem.net/doc/natives_cfx.lua
curl -z in\natives_rdr3.lua -Lo in\natives_rdr3_new.lua http://runtime.fivem.net/doc/natives_rdr_tmp.lua
::curl -Lo in\natives_global_new.lua http://runtime.fivem.net/doc/natives_rdr_tmp.lua
::curl -z in\natives_cfx.lua -Lo in\natives_cfx_new.lua http://runtime.fivem.net/doc/natives_cfx.lua
echo -- ok > in\natives_cfx_new.lua

if exist in\natives_cfx.lua (
	diff in\natives_cfx.lua in\natives_cfx_new.lua > nul
	
	if errorlevel 0 (
		copy /y in\natives_cfx_new.lua in\natives_cfx.lua
	)
) else (
	copy /y in\natives_cfx_new.lua in\natives_cfx.lua
)

if exist in\natives_global.lua (
	diff in\natives_global.lua in\natives_global_new.lua > nul
	
	if errorlevel 0 (
		copy /y in\natives_global_new.lua in\natives_global.lua
	)
) else (
	copy /y in\natives_global_new.lua in\natives_global.lua
)

if exist in\natives_rdr3.lua (
	diff in\natives_rdr3.lua in\natives_rdr3_new.lua > nul
	
	if errorlevel 0 (
		copy /y in\natives_rdr3_new.lua in\natives_rdr3.lua
	)
) else (
	copy /y in\natives_rdr3_new.lua in\natives_rdr3.lua
)

del in\natives_global_new.lua
del in\natives_cfx_new.lua
del in\natives_rdr3_new.lua

make -q

if errorlevel 1 (
	make -j4
	xcopy /y out\*.lua ..\..\data\shared\citizen\scripting\lua
	xcopy /y out\*.js ..\..\data\shared\citizen\scripting\v8
	xcopy /y out\*.d.ts ..\..\data\shared\citizen\scripting\v8
	xcopy /y out\rpc_natives.json ..\..\data\shared\citizen\scripting
	xcopy /y out\*.zip ..\..\data\shared\citizen\scripting\lua

	xcopy /y out\*.cs ..\..\code\client\clrcore
	xcopy /y out\*.h ..\..\code\components\citizen-scripting-lua\src
)

popd
