@echo off

where /q node
if errorlevel 1 (
    goto :error
)

if "%1" == "chat" (
    goto :chat 
)

set SRRoot=%~dp0\data
pushd %~dp0


:: txAdmin
echo Adding monitor

set MonitorPath=%SRRoot%\monitor
set MonitorArtifactPath=%SRRoot%\monitor.zip

rmdir /s /q %MonitorPath%
mkdir %MonitorPath%

node manager.js download --name=monitor --file=%MonitorArtifactPath% || goto :error

tar -C %MonitorPath%\ -xf %MonitorArtifactPath% || goto :error
del %MonitorArtifactPath%

echo Done adding monitor
:: /txAdmin


:: chat
:chat
echo Adding chat

pushd resources\chat
rmdir /s /q dist

node %~dp0\..\native-doc-gen\yarn_cli.js || goto :error

set NODE_OPTIONS=""
FOR /F %%g IN ('node -v') do (set NODE_VERSION_STRING=%%g)
set /a NODE_VERSION="%NODE_VERSION_STRING:~1,2%"
IF %NODE_VERSION% GEQ 18 (set NODE_OPTIONS=--openssl-legacy-provider)

call node_modules\.bin\webpack.cmd || goto :error
popd

rmdir /s /q %SRRoot%\chat\

rmdir /s /q resources\chat\node_modules\

xcopy /y /e resources\chat %SRRoot%\chat\ || goto :error
del %SRRoot%\chat\yarn.lock
del %SRRoot%\chat\package.json

rmdir /s /q %SRRoot%\chat\html\
mkdir %SRRoot%\chat\html\vendor\

xcopy /y /e resources\chat\html\vendor %SRRoot%\chat\html\vendor || goto :error
popd

echo Done adding chat
goto :success
:: /chat


:error
echo Failed to build system resources
exit /b %errorlevel%


:success
exit /b 0
