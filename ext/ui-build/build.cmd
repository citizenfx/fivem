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
call node_modules\.bin\ng.cmd build --prod

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

mkdir %~dp0\data_big\app
move /y %UIRoot%\app\*.jpg %~dp0\data_big\app\

del %~dp0\data.zip
%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data.zip %UIRoot%\*

del %~dp0\data_big.zip

%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data_big.zip %~dp0\data_big\*

exit /B 0