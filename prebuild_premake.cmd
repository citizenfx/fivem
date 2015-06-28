@echo off
powershell -executionpolicy bypass -file citizenmp\prebuild_premake.ps1

pushd build\premake
call git submodule update --init --recursive
..\premake5 vs2015
..\premake5 embed
popd

call "%VCINSTALLDIR%\vcvarsall.bat" amd64_x86
set PLATFORM=
msbuild build/premake/Premake5.sln /p:configuration=release
