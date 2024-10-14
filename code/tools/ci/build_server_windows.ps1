using module .\psm1\cfxBuildContext.psm1
using module .\psm1\cfxBuildSystemResources.psm1
using module .\psm1\cfxBuildTools.psm1
using module .\psm1\cfxCreateGitTag.psm1
using module .\psm1\cfxGitlabSections.psm1
using module .\psm1\cfxPackServer.psm1
using module .\psm1\cfxRunMSBuild.psm1
using module .\psm1\cfxRunPrebuild.psm1
using module .\psm1\cfxRunPremake.psm1
using module .\psm1\cfxSetupPrivate.psm1
using module .\psm1\cfxSetupSubmodules.psm1
using module .\psm1\cfxSetupVS.psm1
using module .\psm1\cfxSentry.psm1
using module .\psm1\cfxUpdateVersionFiles.psm1
using module .\psm1\cfxUploadServerSymbols.psm1
using module .\psm1\cfxVersions.psm1

try {
    $ctx = Get-CfxBuildContext -RequireProductName
    $tools = Get-CfxBuildTools -Context $ctx
    $versions = Get-CfxVersions -Context $ctx
    
    $tools.ensurePython()
    $tools.ensureNodeJS()

    Write-Output "Context:", $ctx, "`n"
    Write-Output "Tools:", $tools, "`n"
    Write-Output "Versions:", $versions, "`n"

    $ctx.startBuild()

    if ($ctx.IsRetailBuild) {
        Invoke-LogSection "Creating tag" {
            Invoke-CreateGitTag -Context $ctx -Versions $versions
        }.GetNewClosure()
    }

    Invoke-LogSection "Visual Studio Environment Setup" {
        Invoke-CfxSetupVS -Context $ctx -Tools $tools
    }.GetNewClosure()

    Invoke-LogSection "Submodules Setup" {
        Invoke-SetupSubmodules -Context $ctx
    }.GetNewClosure()

    Invoke-LogSection "Running prebuild" {
        Invoke-RunPrebuild -Context $ctx
    }.GetNewClosure()

    Invoke-CfxSetupPrivate -Context $ctx -Versions $versions

    Invoke-LogSection "Running premake" {
        Invoke-RunPremake -Context $ctx -Tools $tools
    }.GetNewClosure()

    Invoke-LogSection "Updating version files" {
        Invoke-UpdateVersionFiles -Context $ctx -Versions $versions
    }.GetNewClosure()

    Invoke-LogSection "Running MSBuild" {
        Invoke-RunMSBuild -Context $ctx -Tools $tools
    }.GetNewClosure()

    Invoke-LogSection "Uploading symbols" {
        Invoke-UploadServerSymbols -Context $ctx -Tools $tools
    }.GetNewClosure()

    Invoke-LogSection "Building system resources" {
        Invoke-BuildSystemResources -Context $ctx -Versions $versions
    }.GetNewClosure()

    Invoke-LogSection "Packing server" {
        Invoke-PackServer -Context $ctx -Tools $tools
    }.GetNewClosure()

    if ($ctx.IsPublicBuild){
        Invoke-LogSection "Creating sentry release" {
            Invoke-SentryCreateRelease -Context $ctx -Version $ctx.GitTag
        }.GetNewClosure()
    }

    $ctx.finishBuild()
} catch {
    Write-Output $_
    Write-Output "BUILD FAILED"

    Exit 1
}
