@echo off

setlocal EnableDelayedExpansion

set path=C:\msys64\usr\bin;%path%
set LUA_PATH=
set LUA_CPATH=

where /q pacman

if errorlevel 1 (
    exit /B 1
)

pacman --noconfirm --needed -Sy make diffutils

where /q curl

if errorlevel 1 (
    exit /B 1
)

pushd ext\natives\
mkdir inp

call:UpdateToLatest inp\natives_global.lua https://runtime.fivem.net/doc/natives.lua
call:UpdateToLatest inp\natives_rdr3.lua https://runtime.fivem.net/doc/natives_rdr3.lua
call:UpdateToLatest inp\natives_rdr3_old.lua https://runtime.fivem.net/doc/natives_rdr_tmp.lua
call:UpdateToLatest inp\natives_ny.lua https://runtime.fivem.net/doc/natives_ny_tmp.lua

call:UpdateToLatest inp\natives_global_client_compat.lua https://runtime.fivem.net/doc/natives_global_client_compat.lua
call:UpdateToLatest inp\natives_rdr3_client_compat.lua https://runtime.fivem.net/doc/natives_rdr3_client_compat.lua

popd

pushd ext\native-doc-gen\
sh build.sh

if errorlevel 1 (
	popd
	exit /B 1
)

popd

pushd ext\natives\
make -q

if errorlevel 1 (
	make -j4
)

:: Always copy files, they're within the build cache and it's easier to copy them and not to cache their copies in destination folders
:: luckily xcopy preserves the file timestamps and it shouldn't normally trigger recompilation of dependant components
xcopy /y out\*.lua ..\..\data\shared\citizen\scripting\lua
xcopy /y out\*.js ..\..\data\shared\citizen\scripting\v8
xcopy /y out\*.d.ts ..\..\data\shared\citizen\scripting\v8
xcopy /y out\rpc_natives.json ..\..\data\shared\citizen\scripting
xcopy /y out\*.zip ..\..\data\shared\citizen\scripting\lua

xcopy /y out\*.cs ..\..\code\client\clrcore
xcopy /y out\v2\*.cs ..\..\code\client\clrcore-v2\Native
xcopy /y out\Natives*.h ..\..\code\components\citizen-scripting-lua\src
xcopy /y out\NativeTypes*.h ..\..\code\components\citizen-scripting-core\src

popd
goto :eof

:: Functions

:UpdateToLatest
echo Updating %~1
%systemroot%\system32\curl -fz %~1 -Lo %~1.new %~2

if not errorlevel 0 (
	echo 	cURL exited with error code %errorlevel%.
) else (
	if exist %~1.new (
		if exist %~1 (
			diff %~1 %~1.new > nul

			if not errorlevel 1 (
				del %~1.new
				exit /B 0
			)
		)
		
		move /y %~1.new %~1
	) else (
		echo 	File is up-to-date.
	)
)

exit /B 0
