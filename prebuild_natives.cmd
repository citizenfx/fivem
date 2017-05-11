@echo off

pushd ext\natives\
powershell -executionpolicy unrestricted .\generate_natives.ps1
copy /y out\* ..\..\data\citizen\scripting\lua
popd
