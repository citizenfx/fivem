@echo off

:: check if Yarn exists

where /q yarn

if errorlevel 1 (
    exit /B 1
)

set CacheRoot=C:\f\save

:: build sdk-root
set SDKRoot=%~dp0\sdk-root

:: push directory
pushd ..\sdk\resources\sdk-root\host

:: copy sdk-root node_modules from cache
if exist %CacheRoot%\sdk-root-modules (
	rmdir /s /q node_modules
	move %CacheRoot%\sdk-root-modules node_modules
)

:: install packages (using Yarn now)
call yarn

:: build host
call yarn build

:: move theia node_modules out of resource so we can then make fast copy
move %SDKRoot%\resource\host\personality-theia\node_modules %SDKRoot%\sdk-root-personality-theia-modules
:: delete old build
rmdir /s /q %SDKRoot%\resource\
:: returning theia node_modules back
mkdir %SDKRoot%\resource\host\personality-theia\
move %SDKRoot%\sdk-root-personality-theia-modules %SDKRoot%\resource\host\personality-theia\node_modules

:: copy shell
mkdir %SDKRoot%\resource\host\shell\
xcopy /y /e shell\build\*.* %SDKRoot%\resource\host\shell\

:: fast copy theia node_modules, if needed
robocopy personality-theia\node_modules\ %SDKRoot%\resource\host\personality-theia\node_modules\ /mir /xd electron* /xd fxdk-app /xo /fft /ndl /njh /njs /nc /ns /np

xcopy /y /e personality-theia\fxdk-app\lib\*.* %SDKRoot%\resource\host\personality-theia\lib\
xcopy /y /e personality-theia\fxdk-app\plugins\*.* %SDKRoot%\resource\host\personality-theia\plugins\

xcopy /y personality-theia\fxdk-app\package.json %SDKRoot%\resource\host\personality-theia\
xcopy /y personality-theia\fxdk-app\server.js %SDKRoot%\resource\host\personality-theia\

:: copy resouce files
xcopy /y ..\fxmanifest.lua %SDKRoot%\resource\
xcopy /y ..\sdk.js %SDKRoot%\resource\
xcopy /y ..\mime-types.js %SDKRoot%\resource\

if exist %CacheRoot% (
	rmdir /s /q %CacheRoot%\sdk-root-modules
	move node_modules %CacheRoot%\sdk-root-modules
)

:: pop directory
popd

exit /B 0
