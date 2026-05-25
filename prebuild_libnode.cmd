@echo off

call:UpdateToLatestLibnode vendor\libnode\bin\libnode22.dll https://github.com/citizenfx/libnode/releases/latest/download/libnode22.dll
call:UpdateToLatestLibnode vendor\libnode\bin\libnode22.lib https://github.com/citizenfx/libnode/releases/latest/download/libnode22.lib
call:UpdateToLatestLibnode vendor\libnode\bin\libnode22.pdb https://github.com/citizenfx/libnode/releases/latest/download/libnode22.pdb
call:UpdateToLatestLibnode vendor\libnode\bin\libnode22.so https://github.com/citizenfx/libnode/releases/latest/download/libnode22.so
call:UpdateToLatestLibnode vendor\libnode\bin\libuv.dll https://github.com/citizenfx/libnode/releases/latest/download/libuv.dll
call:UpdateToLatestLibnode vendor\libnode\bin\libuv.lib https://github.com/citizenfx/libnode/releases/latest/download/libuv.lib
call:UpdateToLatestLibnode vendor\libnode\bin\libuv.pdb https://github.com/citizenfx/libnode/releases/latest/download/libuv.pdb
call:UpdateToLatestLibnode vendor\libnode\bin\libuv.so https://github.com/citizenfx/libnode/releases/latest/download/libuv.so

goto :eof

:UpdateToLatestLibnode
echo Updating %~1
%systemroot%\system32\curl --ssl-no-revoke -fz %~1 -Lo %~1.new %~2

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
