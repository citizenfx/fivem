@echo off
call code\prebuild_udis86.cmd
call code\prebuild_misc.cmd

cd %~dp0

pushd ext\natives\
powershell -executionpolicy unrestricted .\generate_natives.ps1
copy /y out\* ..\..\data\citizen\scripting\lua
popd
