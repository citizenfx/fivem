@echo off

:: check if Yarn exists

where /q yarn

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

:: install packages (using Yarn now)
call yarn

:: workaround for duplicate webpack versions
rmdir /s /q node_modules\@angular-devkit\build-angular\node_modules\webpack

:: build the worker
:: unused now
::call node_modules\.bin\webpack.cmd --config=worker.config.js

:: ng build
call node_modules\.bin\ng.cmd build --prod 2>&1

:: delete old app
rmdir /s /q %UIRoot%\app\

:: copy new app
mkdir %UIRoot%\app\
xcopy /y /e dist\*.* %UIRoot%\app\

if exist %CacheRoot% (
	rmdir /s /q %CacheRoot%\cfx-ui-modules
	move node_modules %CacheRoot%\cfx-ui-modules
)

:: pop directory
popd

mkdir %~dp0\data_big\app\assets\images
move /y %UIRoot%\app\assets\images\bg*.* %~dp0\data_big\app\assets\images

del %~dp0\data.zip
%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data.zip %UIRoot%\*

del %~dp0\data_big.zip

%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data_big.zip %~dp0\data_big\*

exit /B 0