@echo off

where /q dotnet

if errorlevel 1 (
    exit /B 1
)

where /q node

if errorlevel 1 (
    exit /B 1
)

set SRRoot=%~dp0\data

pushd ..\webadmin\server
dotnet publish -c Release
popd

mkdir %SRRoot%\webadmin\wwwroot\
mkdir %SRRoot%\webadmin\server\bin\Release\netstandard2.0\publish\
copy /y ..\webadmin\fxmanifest.lua %SRRoot%\webadmin\

xcopy /y /e ..\webadmin\wwwroot\. %SRRoot%\webadmin\wwwroot\
xcopy /y /e ..\webadmin\server\bin\Release\netstandard2.0\publish\. %SRRoot%\webadmin\server\bin\Release\netstandard2.0\publish\

pushd ..\txAdmin
call npm i
call node_modules\.bin\webpack.cmd --config webpack.config.js --progress 2>&1 | findstr /V "not found"
popd

rmdir /s /q %SRRoot%\monitor\
mkdir %SRRoot%\monitor\

xcopy /y /e ..\txAdmin\dist %SRRoot%\monitor

exit /B 0