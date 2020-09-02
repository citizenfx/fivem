@echo off

:: check if Yarn exists

where /q yarn

if errorlevel 1 (
    exit /B 1
)

set CacheRoot=C:\f\save

:: build sdk-root
set UIRoot=%~dp0\sdk-root

:: push directory
pushd ..\sdk\resources\sdk-root\app

:: copy sdk-root node_modules from cache
if exist %CacheRoot%\sdk-root-modules (
	rmdir /s /q node_modules
	move %CacheRoot%\sdk-root-modules node_modules
)

:: install packages (using Yarn now)
call yarn

:: build the app
call yarn build

:: delete old app
rmdir /s /q %UIRoot%\app\

:: copy new app
mkdir %UIRoot%\resource\app\
xcopy /y /e build\*.* %UIRoot%\resource\app\

:: copy resouce files
xcopy /y ..\fxmanifest.lua %UIRoot%\resource\
xcopy /y ..\sdk.js %UIRoot%\resource\
xcopy /y ..\mime-types.js %UIRoot%\resource\

if exist %CacheRoot% (
	rmdir /s /q %CacheRoot%\sdk-root-modules
	move node_modules %CacheRoot%\sdk-root-modules
)

:: pop directory
popd

exit /B 0
