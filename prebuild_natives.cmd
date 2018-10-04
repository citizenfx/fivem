@echo off

set path=C:\msys64\usr\bin;%path%

pacman --noconfirm --needed -S make curl

pushd ext\natives\
curl -sLo out\natives_global.lua http://runtime.fivem.net/doc/natives.lua
curl -sLo out\natives_cfx.lua http://runtime.fivem.net/doc/natives_cfx.lua
make -j4
xcopy /y out\*.lua ..\..\data\shared\citizen\scripting\lua
xcopy /y out\*.js ..\..\data\shared\citizen\scripting\v8
xcopy /y out\*.d.ts ..\..\data\shared\citizen\scripting\v8
xcopy /y out\rpc_natives.json ..\..\data\shared\citizen\scripting
xcopy /y out\*.zip ..\..\data\shared\citizen\scripting\lua

xcopy /y out\*.cs ..\..\code\client\clrcore
popd
