using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1

function Invoke-PackServer {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools
    )

    $binRoot = $Context.MSBuildOutput
    $packRoot = $Context.getPathInProject("out")
    $projectRoot = $Context.ProjectRoot

    Remove-Item -Recurse -Force -ErrorAction Ignore $packRoot | Out-Null

    # create folders layout
    New-Item -ItemType Directory -Force $packRoot\server\citizen\system_resources | Out-Null

    # copy system resources
    Copy-Item -Force -Recurse $projectRoot\ext\system-resources\data\* $packRoot\server\citizen\system_resources\

    # excludes the unit test runner CitiTest.exe, because it doesn't need to be distributed by the ci
    Copy-Item -Force -Exclude "CitiTest.exe" $binRoot\*.exe $packRoot\server\
    Copy-Item -Force $binRoot\*.dll $packRoot\server\

    Copy-Item -Force -Recurse $binRoot\citizen\* $packRoot\server\citizen\

    Copy-Item -Force -Recurse $projectRoot\data\shared\*         $packRoot\server\
    Copy-Item -Force -Recurse $projectRoot\data\redist\crt\*     $packRoot\server\
    Copy-Item -Force -Recurse $projectRoot\data\server\*         $packRoot\server\
    Copy-Item -Force -Recurse $projectRoot\data\server_windows\* $packRoot\server\
    Copy-Item -Force -Recurse $projectRoot\data\server_windows_release\* $packRoot\server\

    Remove-Item -Force $packRoot\server\citizen\.gitignore

    # breaks downlevel OS compat
    Remove-Item -Force $packRoot\server\dbghelp.dll

    # cleanup client-related scripting files
    Remove-Item -Force $packRoot\server\citizen\scripting\lua\natives_0*.zip
    Remove-Item -Force $packRoot\server\citizen\scripting\lua\natives_2*.zip
    Remove-Item -Force $packRoot\server\citizen\scripting\lua\*_universal.lua
    Remove-Item -Force $packRoot\server\citizen\scripting\lua\*_universal.zip
    Remove-Item -Force $packRoot\server\citizen\scripting\lua\natives_0*.lua
    Remove-Item -Force $packRoot\server\citizen\scripting\lua\natives_2*.lua

    Remove-Item -Force $packRoot\server\citizen\scripting\v8\*_universal.d.ts
    Remove-Item -Force $packRoot\server\citizen\scripting\v8\*_universal.js
    Remove-Item -Force $packRoot\server\citizen\scripting\v8\natives_0*.*
    Remove-Item -Force $packRoot\server\citizen\scripting\v8\natives_2*.*

    & $Tools.SevenZip a -mx=9 $packRoot\server.zip $packRoot\server\*
    & $Tools.SevenZip a -mx=7 $packRoot\server.7z $packRoot\server\*

    # cleanup
    Remove-Item -Force -Recurse $packRoot\server
}