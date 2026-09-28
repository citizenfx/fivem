@echo off
SETLOCAL EnableDelayedExpansion

set GAME=%1
SET UI_BUNDLE=cfx-ui-%GAME%.zip
SET UI_BIG_BUNDLE=cfx-ui-%GAME%_big.zip

set URL_FIVE=https://downloads.cfx-services.net/prod/01a0cd92-076d-71b1-8d35-c60751687baf/cfx-ui-five.zip
set URL_FIVE_BIG=https://downloads.cfx-services.net/prod/01a0cd92-0f92-7f10-90bb-28a0c05aa777/cfx-ui-five_big.zip
set URL_RDR3=https://downloads.cfx-services.net/prod/01a0cd92-137c-7815-98b7-bc7cf5fb81dc/cfx-ui-rdr3.zip
set URL_RDR3_BIG=https://downloads.cfx-services.net/prod/01a0cd92-168c-75b4-a700-a06c78d7840b/cfx-ui-rdr3_big.zip

if "%GAME%"=="five" (
    set UI_URL=%URL_FIVE%
    set UI_BIG_URL=%URL_FIVE_BIG%
) else if "%GAME%"=="rdr3" (
    set UI_URL=%URL_RDR3%
    set UI_BIG_URL=%URL_RDR3_BIG%
) else (
    echo Invalid game specified: %GAME%
    exit /b 1
)

:: check if Yarn exists

where /q yarn || exit /b !ERRORLEVEL!

:: build loading screen
echo Building loading screen...
pushd loadscreen
call yarn || exit /b !ERRORLEVEL!
call node_modules\.bin\webpack || exit /b !ERRORLEVEL!

echo Copying loadscreen files...
xcopy /y /e dist\*.* %~dp0\data\loadscreen\ || exit /b !ERRORLEVEL!
popd

:: make sure no app leftovers
if exist %~dp0\data\app (
    rmdir /s /q %~dp0\data\app\
)
if exist %~dp0\data_big\app (
    rmdir /s /q %~dp0\data_big\app\
)

echo Moving loadscreen large files to data_big...
mkdir %~dp0\data_big\loadscreen
move /y %~dp0\data\loadscreen\*.jpg %~dp0\data_big\loadscreen\

powershell -ExecutionPolicy Unrestricted .\make_dates.ps1 %~dp0\data
powershell -ExecutionPolicy Unrestricted .\make_dates.ps1 %~dp0\data_big

if exist %~dp0\data.zip (
    del %~dp0\data.zip
)
if exist %~dp0\data_big.zip (
    del %~dp0\data_big.zip
)

:: download UI bundles
echo Downloading UI bundle...
curl.exe -k --fail-with-body -z%UI_BUNDLE% -L -o%UI_BUNDLE% %UI_URL% || exit /b !ERRORLEVEL!
echo Downloading UI big bundle...
curl.exe -k --fail-with-body -z%UI_BIG_BUNDLE% -L -o%UI_BIG_BUNDLE% %UI_BIG_URL% || exit /b !ERRORLEVEL!

copy /y %~dp0\%UI_BUNDLE% %~dp0\data.zip || exit /b !ERRORLEVEL!
copy /y %~dp0\%UI_BIG_BUNDLE% %~dp0\data_big.zip || exit /b !ERRORLEVEL!

%~dp0\..\..\code\tools\ci\7z u -mx=0 %~dp0\data.zip %~dp0\data\* || exit /b !ERRORLEVEL!
%~dp0\..\..\code\tools\ci\7z u -mx=0 %~dp0\data_big.zip %~dp0\data_big\* || exit /b !ERRORLEVEL!

exit /B 0