mkdir -Force out

foreach ($file in (Get-Item natives_stash\*.lua)) {
    remove-item -force out\natives\*.lua
    mkdir -force out\natives

    .\lua53 codegen.lua $file.FullName | out-file -encoding ascii "out\natives_$(($file.BaseName -replace "gta_", '').ToLower()).lua"
    .\lua53 codegen.lua $file.FullName slua | out-null
    .\lua53 codegen.lua $file.FullName js | out-file -encoding ascii "out\natives_$(($file.BaseName -replace "gta_", '').ToLower()).js"
    .\lua53 codegen.lua $file.FullName dts | out-file -encoding ascii "out\natives_$(($file.BaseName -replace "gta_", '').ToLower()).d.ts"

    Push-Location out\natives\
    ..\..\..\..\code\tools\ci\7z.exe a "..\natives_$(($file.BaseName -replace "gta_", '').ToLower()).zip"  *.lua
    Pop-Location
}

.\lua53 codegen.lua natives_stash\gta_universal.lua lua server | out-file -encoding ascii "out\natives_server.lua"

# write NativesFive.cs
"#if GTA_FIVE
namespace CitizenFX.Core.Native
{
" | out-file -encoding ascii "..\..\code\client\clrcore\NativesFive.cs"

.\lua53 codegen.lua natives_stash\gta_universal.lua enum | out-file -append -encoding ascii "..\..\code\client\clrcore\NativesFive.cs"
.\lua53 codegen.lua natives_stash\gta_universal.lua cs | out-file -append -encoding ascii "..\..\code\client\clrcore\NativesFive.cs"

"}
#endif
" | out-file -append -encoding ascii "..\..\code\client\clrcore\NativesFive.cs"

# write NativesServer.cs
"#if IS_FXSERVER
namespace CitizenFX.Core.Native
{
" | out-file -encoding ascii "..\..\code\client\clrcore\NativesServer.cs"

.\lua53 codegen.lua natives_stash\gta_universal.lua enum server | out-file -append -encoding ascii "..\..\code\client\clrcore\NativesServer.cs"
.\lua53 codegen.lua natives_stash\gta_universal.lua cs server | out-file -append -encoding ascii "..\..\code\client\clrcore\NativesServer.cs"

"}
#endif
" | out-file -append -encoding ascii "..\..\code\client\clrcore\NativesServer.cs"

.\lua53 codegen.lua .\natives_stash\gta_universal.lua rpc server | out-file -encoding ascii "out\rpc_natives.json"

.\lua53 codegen.lua natives_stash\gta_universal.lua js server | out-file -encoding ascii "out\natives_server.js"
.\lua53 codegen.lua natives_stash\gta_universal.lua dts server | out-file -encoding ascii "out\natives_server.d.ts"