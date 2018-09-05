@echo off

set path=%path%;C:\msys64\usr\bin

pushd ext\natives\
make -j4
xcopy /y out\*.lua ..\..\data\shared\citizen\scripting\lua
xcopy /y out\*.js ..\..\data\shared\citizen\scripting\v8
xcopy /y out\*.d.ts ..\..\data\shared\citizen\scripting\v8
xcopy /y out\rpc_natives.json ..\..\data\shared\citizen\scripting
xcopy /y out\*.zip ..\..\data\shared\citizen\scripting\lua
popd
