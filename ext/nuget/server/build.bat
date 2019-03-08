@echo off
set ROOT=%CD%
cd %~dp0

set NUGET=%CD%\..\..\..\code\tools\ci\nuget.exe

mkdir lib\net45\

%ROOT%\code\tools\ci\7z.exe -o%ROOT%\out\server\ x %ROOT%\out\server.zip

copy /y %ROOT%\out\server\citizen\clr2\lib\mono\4.5\ref\CitizenFX.Core.Server.dll lib\net45\CitizenFX.Core.Server.dll
copy /y %ROOT%\out\server\citizen\clr2\lib\mono\4.5\ref\CitizenFX.Core.Server.xml lib\net45\CitizenFX.Core.Server.xml

%NUGET% pack CitizenFX.Core.Server.nuspec -Exclude build.bat -Version "1.0.%CI_PIPELINE_ID%"
%NUGET% push CitizenFX.Core.Server.1.0.%CI_PIPELINE_ID%.nupkg -ApiKey %NUGET_TOKEN% -Source https://api.nuget.org/v3/index.json

del CitizenFX.Core.Server.1.0.%CI_PIPELINE_ID%.nupkg