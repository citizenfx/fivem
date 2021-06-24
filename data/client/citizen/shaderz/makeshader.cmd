@echo off

call :GetWin10SdkDirHelper HKLM\SOFTWARE\Wow6432Node > nul 2>&1
if errorlevel 1 call :GetWin10SdkDirHelper HKCU\SOFTWARE\Wow6432Node > nul 2>&1
if errorlevel 1 call :GetWin10SdkDirHelper HKLM\SOFTWARE > nul 2>&1
if errorlevel 1 call :GetWin10SdkDirHelper HKCU\SOFTWARE > nul 2>&1
if errorlevel 1 exit /B 1

set DX_FXC=%WindowsSdkDir%bin\%WindowsSDKVersion%x64\fxc.exe

if not exist "%DX_FXC%" (
    echo Could not find fxc.exe
    exit /B 1
)

set CFX_CLI=FiveM.com

where /q FiveM.com

if errorlevel 1 (
    if exist %~dp0\..\..\..\..\code\bin\five\debug\FiveM.com (
        set CFX_CLI=%~dp0\..\..\..\..\code\bin\five\debug\FiveM.com
        goto :yea
    )

    if exist %LOCALAPPDATA%\FiveM\FiveM.app\FiveM.com (
        set CFX_CLI=%LOCALAPPDATA%\FiveM\FiveM.app\FiveM.com
        goto :yea
    )

    if exist %~dp0\..\..\..\..\code\bin\five\release\FiveM.com (
        set CFX_CLI=%~dp0\..\..\..\..\code\bin\five\release\FiveM.com
        goto :yea
    )

    echo Could not find FiveM.com
    exit /B 1
)

:yea
set SHADERPATH=%~p1
set SHADERNAME=%~n1
set SHADERFILE=%1

echo --------------------------------
echo %SHADERFILE%
echo --------------------------------

echo ----------------
echo win32 40 final
echo ----------------

if not exist %SHADERPATH%win32_40_final\* mkdir %SHADERPATH%win32_40_final
"%DX_FXC%" /O2 /T fx_5_0 /nologo /Fl %SHADERPATH%win32_40_final\%SHADERNAME%.fxl %SHADERFILE%
"%CFX_CLI%" formats:compileShaders %SHADERPATH%win32_40_final\%SHADERNAME%.fxl
del /q %SHADERPATH%win32_40_final\%SHADERNAME%.fxl

echo ----------------
echo win32 40 lq final
echo ----------------

if not exist %SHADERPATH%win32_40_lq_final\* mkdir %SHADERPATH%win32_40_lq_final
"%DX_FXC%" /O2 /T fx_5_0 /nologo /Fl %SHADERPATH%win32_40_lq_final\%SHADERNAME%.fxl %SHADERFILE%
"%CFX_CLI%" formats:compileShaders %SHADERPATH%win32_40_lq_final\%SHADERNAME%.fxl
del /q %SHADERPATH%win32_40_lq_final\%SHADERNAME%.fxl

exit /B 0

:GetWin10SdkDirHelper

@REM Get Windows 10 SDK installed folder
for /F "tokens=1,2*" %%i in ('reg query "%1\Microsoft\Microsoft SDKs\Windows\v10.0" /v "InstallationFolder"') DO (
    if "%%i"=="InstallationFolder" (
        SET WindowsSdkDir=%%~k
    )
)

@REM get windows 10 sdk version number
setlocal enableDelayedExpansion

@REM Due to the SDK installer changes beginning with the 10.0.15063.0 (RS2 SDK), there is a chance that the
@REM Windows SDK installed may not have the full set of bits required for all application scenarios.
@REM We check for the existence of a file we know to be included in the "App" and "Desktop" portions
@REM of the Windows SDK, depending on the Developer Command Prompt's -app_platform configuration.
@REM If "windows.h" (UWP) or "winsdkver.h" (Desktop) are not found, the directory will be skipped as
@REM a candidate default value for [WindowsSdkDir].
set __check_file=winsdkver.h

if not "%WindowsSdkDir%"=="" for /f %%i IN ('dir "%WindowsSdkDir%include\" /b /ad-h /on') DO (
    @REM Skip if Windows.h|winsdkver (based upon -app_platform configuration) is not found in %%i\um.  
    if EXIST "%WindowsSdkDir%include\%%i\um\%__check_file%" (
        set result=%%i
        if "!result:~0,3!"=="10." (
            set SDK=!result!
        )
    )
)

endlocal & set WindowsSDKVersion=%SDK%\

if "%WindowsSdkDir%"=="" (
    exit /B 1
)

exit /B 0