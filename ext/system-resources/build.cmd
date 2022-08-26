@echo off

where /q node

if errorlevel 1 (
    exit /B 1
)

set SRRoot=%~dp0\data

pushd ..\txAdmin
rmdir /s /q dist

call npm install npm@8.13.2
call node_modules\.bin\npm ci
call node_modules\.bin\npm run build 2>&1 | findstr /V "not found"
popd

rmdir /s /q %SRRoot%\monitor\
mkdir %SRRoot%\monitor\

xcopy /y /e ..\txAdmin\dist %SRRoot%\monitor

exit /B 0