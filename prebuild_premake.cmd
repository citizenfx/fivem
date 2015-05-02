@echo off
powershell -executionpolicy bypass -file citizenmp\prebuild_premake.ps1

pushd build\premake
call git submodule update --init --recursive
..\premake4 vs2013
..\premake4 embed
popd

call "%VCINSTALLDIR%\vcvarsall.bat" amd64_x86
set PLATFORM=
msbuild build/premake/Premake5.sln /p:configuration=release
