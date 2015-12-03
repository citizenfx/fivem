@echo off
rem powershell -executionpolicy bypass -file citizenmp\prebuild_premake.ps1

if exist build\premake\bin\release\premake5.exe goto dontbuild

pushd build\premake
call git submodule update --init --recursive
rem ..\premake5 vs2015
rem ..\premake5 embed
del src\host\scripts.c
nmake /f Bootstrap.mak windows
popd

:dontbuild

rem call "%VCINSTALLDIR%\vcvarsall.bat" amd64_x86
rem set PLATFORM=
rem msbuild build/premake/Premake5.sln /p:configuration=release
