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

:: propagate error
if %ERRORLEVEL% neq 0 exit /b 1

:: build the worker
:: unused now
::call node_modules\.bin\webpack.cmd --config=worker.config.js

:: ng build
call node_modules\.bin\ng.cmd build --configuration production 2>&1

:: propagate error
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

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

:: build loading screen
pushd loadscreen
call yarn
if %ERRORLEVEL% neq 0 exit /b 1

call node_modules\.bin\webpack
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

xcopy /y /e dist\*.* %UIRoot%\loadscreen\
popd

mkdir %~dp0\data_big\app\
move /y %UIRoot%\app\bg*.* %~dp0\data_big\app\
move /y %UIRoot%\app\*.svg %~dp0\data_big\app\
move /y %UIRoot%\app\*.woff %~dp0\data_big\app\
move /y %UIRoot%\app\*.woff2 %~dp0\data_big\app\
move /y %UIRoot%\loadscreen\*.jpg %~dp0\data_big\loadscreen\

powershell -ExecutionPolicy Unrestricted .\make_dates.ps1 %~dp0\data
powershell -ExecutionPolicy Unrestricted .\make_dates.ps1 %~dp0\data_big

del %~dp0\data.zip
%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data.zip %UIRoot%\*

del %~dp0\data_big.zip
%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data_big.zip %~dp0\data_big\*

exit /B 0