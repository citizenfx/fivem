@echo off

:: check if NPM exists

where /q npm

if errorlevel 1 (
    exit /B 1
)

:: build UI
set UIRoot=%~dp0\data

:: push directory
pushd ..\cfx-ui\

:: install npm stuff
call npm i

:: ng build
call node_modules\.bin\ng.cmd build --prod --output-hashing none

:: delete old app
rmdir /s /q %UIRoot%\app\

:: copy new app
mkdir %UIRoot%\app\
copy /y dist\*.* %UIRoot%\app\

:: pop directory
popd

exit /B 0