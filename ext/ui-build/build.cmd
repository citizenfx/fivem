@echo off

:: check if Yarn exists

where /q yarn

if errorlevel 1 (
    exit /B 1
)

@REM set CacheRoot=C:\f\save

:: build UI
set UIRoot=%~dp0\data

:: push directory
pushd ..\cfx-ui\

:: install packages (using Yarn now)
call yarn --ignore-engines --frozen-lockfile
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

:: move new app
move /y build\mpMenu %UIRoot%\app

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
move /y %UIRoot%\app\static\media\*.* %~dp0\data_big\app\static\media\
move /y %UIRoot%\loadscreen\*.jpg %~dp0\data_big\loadscreen\

powershell -ExecutionPolicy Unrestricted .\make_dates.ps1 %~dp0\data
powershell -ExecutionPolicy Unrestricted .\make_dates.ps1 %~dp0\data_big

del %~dp0\data.zip
%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data.zip %UIRoot%\*

del %~dp0\data_big.zip
%~dp0\..\..\code\tools\ci\7z a -mx=0 %~dp0\data_big.zip %~dp0\data_big\*

exit /B 0