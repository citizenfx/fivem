@echo off

setlocal EnableDelayedExpansion

set path=C:\msys64\usr\bin;%path%

pacman --noconfirm --needed -S make curl diffutils

pushd ext\natives\
mkdir out
curl -z out\natives_global.lua -Lo out\natives_global_new.lua http://runtime.fivem.net/doc/natives.lua
curl -z out\natives_cfx.lua -Lo out\natives_cfx_new.lua http://runtime.fivem.net/doc/natives_cfx.lua

if exist out\natives_cfx.lua (
	diff out\natives_cfx.lua out\natives_cfx_new.lua > nul
	
	if errorlevel 0 (
		copy /y out\natives_cfx_new.lua out\natives_cfx.lua
	)
) else (
	copy /y out\natives_cfx_new.lua out\natives_cfx.lua
)

if exist out\natives_global.lua (
	diff out\natives_global.lua out\natives_global_new.lua > nul
	
	if errorlevel 0 (
		copy /y out\natives_global_new.lua out\natives_global.lua
	)
) else (
	copy /y out\natives_global_new.lua out\natives_global.lua
)

del out\natives_global_new.lua
del out\natives_cfx_new.lua

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
