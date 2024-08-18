using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1

function Invoke-RunPrebuild {
    param(
        [CfxBuildContext] $Context
    )

    Push-Location $Context.ProjectRoot
        cmd /c .\prebuild.cmd
        Test-LastExitCode "Prebuild failed"
    Pop-Location
}