using module .\psm1\cfxBuildContext.psm1
using module .\psm1\cfxBuildSDK.psm1
using module .\psm1\cfxBuildTools.psm1
using module .\psm1\cfxGitlabSections.psm1
using module .\psm1\cfxSetupSubmodules.psm1
using module .\psm1\cfxVersions.psm1

try {
    $ctx = Get-CfxBuildContext
    $tools = Get-CfxBuildTools -Context $ctx
    $versions = Get-CfxVersions -Context $ctx

    Write-Output "Context:", $ctx, "`n"
    Write-Output "Tools:", $tools, "`n"
    Write-Output "Versions:", $versions, "`n"

    $ctx.startBuild()

    Invoke-LogSection "Submodules Setup" {
        Invoke-SetupSubmodules -Context $ctx
    }.GetNewClosure()

    Invoke-LogSection "Building SDK" {
        Invoke-BuildSDK -Context $ctx -Tools $tools -Versions $versions
    }.GetNewClosure()

    $ctx.finishBuild()
} catch {
    Write-Output $_
    Write-Output "BUILD FAILED"

    Exit 1
}
