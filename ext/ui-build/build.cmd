@echo off

:: check if NPM exists

where /q npm

if errorlevel 1 (
    exit /B 1
)

set CacheRoot=C:\f\save

:: build UI
set UIRoot=%~dp0\data

:: push directory
pushd ..\cfx-ui\

:: copy cfx-ui node_modules from cache
if exist %CacheRoot%\cfx-ui-modules (
	rmdir /s /q node_modules
	move %CacheRoot%\cfx-ui-modules node_modules
)

:: install npm stuff
call npm i

:: build the worker
call node_modules\.bin\webpack.cmd --config=worker.config.js

:: ng build
call node_modules\.bin\ng.cmd build --prod --output-hashing none

:: delete old app
rmdir /s /q %UIRoot%\app\

:: copy new app
mkdir %UIRoot%\app\
copy /y dist\*.* %UIRoot%\app\

mkdir %UIRoot%\app\assets\
copy /y dist\assets\*.json %UIRoot%\app\assets\

mkdir %UIRoot%\app\assets\languages\
copy /y dist\assets\languages\*.json %UIRoot%\app\assets\languages\

if exist %CacheRoot% (
	rmdir /s /q %CacheRoot%\cfx-ui-modules
	move node_modules %CacheRoot%\cfx-ui-modules
)

:: pop directory
popd

exit /B 0