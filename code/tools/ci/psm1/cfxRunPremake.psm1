using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1

function Invoke-RunPremake {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools
    )

    # Ensure we have BOOST_ROOT
    if (!$env:BOOST_ROOT -or !(Test-Path $env:BOOST_ROOT)) {
        $cDevPath = "C:\dev\boost_1_71_0"
        $cLibrariesPath = "C:\Libraries\boost_1_71_0"

        if (Test-Path $cDevPath) {
            $env:BOOST_ROOT = $cDevPath
        }
        elseif (Test-Path $cLibrariesPath) {
            $env:BOOST_ROOT = $cLibrariesPath
        }
        else {
            throw "Unable to find BOOST library, make sure you have the correct path in the BOOST_ROOT env var"
        }
    }

    Push-Location $Context.CodeRoot
        & $Tools.premake @(
            $Context.PremakeVSVersion
            "--game={0}" -f $Context.PremakeGameName
            "--bindir={0}" -f $Context.PremakeBinDir
            "--builddir={0}" -f $Context.PremakeBuildDir
        )
        Test-LastExitCode "Premake failed"
    Pop-Location
}