@echo off

:: check if Yarn exists
where /q yarn
if errorlevel 1 (
    exit /B 1
)

set CacheRoot=C:\f\save
set BuildRoot=%~dp0\sdk-root

set FXDK=..\sdk\resources
set FXDKGame=..\sdk\resources\sdk-game
set FXDKRoot=..\sdk\resources\sdk-root

set FXDKShell=%FXDKRoot%\shell
set FXDKFXCode=%FXDKRoot%\fxcode


:: build sdk-game
pushd %FXDKGame%
call yarn install --frozen-lockfile
call yarn build
popd
:: /build sdk-game


:: build shell
pushd %FXDKShell%
if exist build        rmdir /s /q build
if exist build_server rmdir /s /q build_server
call yarn install --frozen-lockfile
call yarn build
del /q /f /s "build\static\js\*.map"
del /q /f /s "build\static\js\*.txt"
del /q /f /s "build\static\css\*.map"
del /q /f /s "build_server\*.map"
popd
:: /build shell


:: build fxcode
pushd %FXDKFXCode%

call yarn install --frozen-lockfile --ignore-engines
call yarn download-builtin-extensions
call yarn --cwd fxdk install --frozen-lockfile
call yarn --cwd fxdk rebuild-native-modules
call yarn --cwd fxdk build

%~dp0\..\..\code\tools\ci\7z a -mx=0 fxcode.tar out-fxdk-pkg\*

popd
:: /build fxcode


:: move builds
if exist %BuildRoot%\resource rmdir /s /q %BuildRoot%\resource

xcopy /y %FXDKRoot%\fxmanifest.lua %BuildRoot%\resource\
xcopy /y %FXDKShell%\index.js      %BuildRoot%\resource\shell\
xcopy /y %FXDKShell%\mpMenu.html   %BuildRoot%\resource\shell\

move %FXDKShell%\build                 %BuildRoot%\resource\shell\build
move %FXDKShell%\build_server          %BuildRoot%\resource\shell\build_server
move %FXDKFXCode%\fxcode.tar           %BuildRoot%\resource\fxcode.tar
:: /move builds



exit /B 0
