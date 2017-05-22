@echo off

pushd ext\natives\
powershell -executionpolicy unrestricted .\generate_natives.ps1
copy /y out\* ..\..\data\shared\citizen\scripting\lua
popd
