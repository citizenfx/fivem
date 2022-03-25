@echo off

where /q node

if errorlevel 1 (
    exit /B 1
)

set SRRoot=%~dp0\data

pushd ..\txAdmin
call npm install -g npm@7.19.1
call npm ci
call npm run build 2>&1 | findstr /V "not found"
popd

rmdir /s /q %SRRoot%\monitor\
mkdir %SRRoot%\monitor\

xcopy /y /e ..\txAdmin\dist %SRRoot%\monitor

exit /B 0