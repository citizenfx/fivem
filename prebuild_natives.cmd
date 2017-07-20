@echo off

pushd ext\natives\
powershell -executionpolicy unrestricted .\generate_natives.ps1
xcopy /y out\*.lua ..\..\data\shared\citizen\scripting\lua
xcopy /y out\*.js ..\..\data\shared\citizen\scripting\v8
popd
