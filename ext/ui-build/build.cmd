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
call yarn --ignore-engines
:: propagate error
if %ERRORLEVEL% neq 0 exit /b 1

:: run test
call yarn test
:: propagate error
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: remove old build output
rmdir /s /q build

:: build it
call yarn build 2>&1
:: propagate error
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: delete old app
rmdir /s /q %UIRoot%\app\

:: copy new app
mkdir %UIRoot%\app\
xcopy /y /e build\mpMenu\*.* %UIRoot%\app\

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

rmdir /s /q %~dp0\data_big\app\
mkdir %~dp0\data_big\app\static\media\
move /y %UIRoot%\app\static\media\*bg*.jpg %~dp0\data_big\app\static\media\
move /y %UIRoot%\app\static\media\*bgeditor.png %~dp0\data_big\app\static\media\
move /y %UIRoot%\app\static\media\*.svg %~dp0\data_big\app\static\media\
move /y %UIRoot%\app\static\media\*.woff2 %~dp0\data_big\app\static\media\
move /y %UIRoot%\app\static\media\*.mp3 %~dp0\data_big\app\static\media\
move /y %UIRoot%\loadscreen\*.jpg %~dp0\data_big\loadscreen\

powershell -ExecutionPolicy Unrestricted .\make_dates.ps1 %~dp0\data
powershell -ExecutionPolicy Unrestricted .\make_dates.ps1 %~dp0\data_big

del %~dp0\data.zip
%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data.zip %UIRoot%\*

del %~dp0\data_big.zip
%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data_big.zip %~dp0\data_big\*

exit /B 0