@echo off

where /q dotnet

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

pushd ..\monitor\server
dotnet publish -c Release
popd

mkdir %SRRoot%\monitor\server\bin\Release\netstandard2.0\publish\
copy /y ..\monitor\fxmanifest.lua %SRRoot%\monitor\

xcopy /y /e ..\monitor\server\bin\Release\netstandard2.0\publish\. %SRRoot%\monitor\server\bin\Release\netstandard2.0\publish\

exit /B 0