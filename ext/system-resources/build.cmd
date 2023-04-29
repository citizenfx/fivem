@echo off

where /q node

if errorlevel 1 (
    exit /B 1
)

set SRRoot=%~dp0\data

:: txadmin
pushd ..\txAdmin
rmdir /s /q dist

call npm install npm@8.13.2
call node_modules\.bin\npm ci
call node_modules\.bin\npm run build 2>&1 | findstr /V "not found"
popd

rmdir /s /q %SRRoot%\monitor\
mkdir %SRRoot%\monitor\

xcopy /y /e ..\txAdmin\dist %SRRoot%\monitor

:: chat
pushd resources\chat
rmdir /s /q dist

call npm install yarn@1.22
call node_modules\.bin\yarn

:: TODO: only do this if node 18
set NODE_OPTIONS=--openssl-legacy-provider
call node_modules\.bin\webpack
popd

rmdir /s /q %SRRoot%\chat\

rmdir /s /q resources\chat\node_modules\

xcopy /y /e resources\chat\ %SRRoot%\chat\
del %SRRoot%\chat\yarn.lock
del %SRRoot%\chat\package.json

rmdir /s /q %SRRoot%\chat\html\
mkdir %SRRoot%\chat\html\vendor\

xcopy /y /e resources\chat\html\vendor %SRRoot%\chat\html\vendor

:: done!
exit /B 0