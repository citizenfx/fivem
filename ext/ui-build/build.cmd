@echo off
SETLOCAL EnableDelayedExpansion

set GAME=%1
SET UI_BUNDLE=cfx-ui-%GAME%.zip
SET UI_BIG_BUNDLE=cfx-ui-%GAME%_big.zip

if "%GAME%"=="five" (
    set UI_URL=https://downloads.cfx-services.net/prod/019edfd8-6e25-7e0f-b531-5ad3ffda13bf
    set UI_BIG_URL=https://downloads.cfx-services.net/prod/019edfd8-76e0-780e-8bca-d5e0e103abce
) else if "%GAME%"=="rdr3" (
    set UI_URL=https://downloads.cfx-services.net/prod/019edfd8-7bb3-70ed-a162-a09dafb3e491
    set UI_BIG_URL=https://downloads.cfx-services.net/prod/019edfd8-8490-7978-a8b5-6bccb3b81d16
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