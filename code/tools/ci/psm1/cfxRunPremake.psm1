using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1

function Invoke-RunPremake {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools
    )

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