@echo off
powershell -executionpolicy bypass -file citizenmp\prebuild_premake.ps1

pushd build\premake
..\premake4 vs2013
..\premake4 embed
popd
