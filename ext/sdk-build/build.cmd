@echo off

:: check if Yarn exists
where /q yarn
if errorlevel 1 (
    exit /B 1
)

set CacheRoot=C:\f\save
set BuildRoot=%~dp0\sdk-root

set FXDK=..\sdk\resources
set FXDKRoot=..\sdk\resources\sdk-root
set FXDKTheia=%FXDKRoot%\host\personality-theia
set FXDKShell=%FXDKRoot%\host\shell
set FXDKSdk=%FXDKRoot%\sdk



:: build sdk
pushd %FXDKSdk%
rmdir /s /q build
call yarn install --frozen-lockfile --ignore-scripts
call yarn build
xcopy /y /e lib\*.*  	 build\lib\
xcopy /y    index.js 	 build\
xcopy /y    package.json build\
xcopy /y    yarn.lock 	 build\
call yarn --cwd build install --frozen-lockfile --ignore-scripts --no-bin-links --production
del build\yarn.lock
popd
:: /build sdk



:: build shell
pushd %FXDKShell%
rmdir /s /q build
call yarn install --frozen-lockfile --ignore-scripts
call yarn build
popd
:: /build shell



:: build theia
pushd %FXDKTheia%
rmdir /s /q build
call yarn install --frozen-lockfile --ignore-scripts
call yarn build
xcopy /y /e fxdk-app\lib\*.* 	    build\lib\
xcopy /y /e fxdk-app\plugins\*.*    build\plugins\
xcopy /y    fxdk-app\backend.js     build\
xcopy /y    yarn.lock               build\

echo F|xcopy /y    fxdk-app\backend-package.json build\package.json
echo F|xcopy /y    build.yarnclean               build\.yarnclean

call yarn --cwd build install --frozen-lockfile --ignore-scripts --no-bin-links --production

xcopy /y /e node_modules\fxdk-project\lib\*.*      build\node_modules\fxdk-project\lib\
xcopy /y    node_modules\fxdk-project\package.json build\node_modules\fxdk-project\

for %%m in (@theia\node-pty, nsfw, find-git-repositories, drivelist) do (
	call yarn electron-rebuild -f -m build\node_modules\%%m
)
call yarn autoclean --force
del /q /f /s "build\*.gz"
del /q /f /s "build\*.map"
del build\yarn.lock
del build\.yarnclean
rmdir /s /q build\node_modules\font-awesome

%~dp0\..\..\code\tools\ci\7z a -mx=0 personality-theia.tar build\*
rmdir /s /q build
popd
:: /build theia



:: move builds
rmdir /s /q %BuildRoot%\resource

mkdir %BuildRoot%\resource\host\

xcopy /y %FXDKRoot%\fxmanifest.lua %BuildRoot%\resource\

move %FXDKSdk%\build                   %BuildRoot%\resource\sdk
move %FXDKShell%\build                 %BuildRoot%\resource\host\shell
move %FXDKTheia%\personality-theia.tar %BuildRoot%\resource\host\personality-theia.tar
:: /move builds



exit /B 0
