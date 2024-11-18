using module .\psm1\cfxBuildContext.psm1
using module .\psm1\cfxBuildTools.psm1
using module .\psm1\cfxBuildUI.psm1
using module .\psm1\cfxGitlabSections.psm1
using module .\psm1\cfxPackClient.psm1
using module .\psm1\cfxRunMSBuild.psm1
using module .\psm1\cfxRunPrebuild.psm1
using module .\psm1\cfxRunPremake.psm1
using module .\psm1\cfxSetupBuildToolkit.psm1
using module .\psm1\cfxSetupCEF.psm1
using module .\psm1\cfxSetupPrivate.psm1
using module .\psm1\cfxSetupSubmodules.psm1
using module .\psm1\cfxSetupVS.psm1
using module .\psm1\cfxSentry.psm1
using module .\psm1\cfxUpdateVersionFiles.psm1
using module .\psm1\cfxUploadClientSymbols.psm1
using module .\psm1\cfxVersions.psm1

try {
    $ctx = Get-CfxBuildContext -RequireProductName
    $tools = Get-CfxBuildTools -Context $ctx
    $versions = Get-CfxVersions -Context $ctx

    $tools.ensurePython()
    $tools.ensureNodeJS()
    $tools.ensureYarn()

    Write-Output "Context:", $ctx, "`n"
    Write-Output "Tools:", $tools, "`n"
    Write-Output "Versions:", $versions, "`n"

    $ctx.startBuild()

    # Invoke-CfxSetupBuildToolkit -Context $ctx

    Invoke-LogSection "Visual Studio Environment Setup" {
        Invoke-CfxSetupVS -Context $ctx -Tools $tools
    }.GetNewClosure()

    Invoke-LogSection "Submodules Setup" {
        Invoke-SetupSubmodules -Context $ctx
    }.GetNewClosure()

    Invoke-LogSection "CEF Setup" {
        Invoke-CfxSetupCEF -Context $ctx -Tools $tools
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
        Invoke-UploadClientSymbols -Context $ctx -Tools $tools
    }.GetNewClosure()

    Invoke-LogSection "Building UI" {
        Invoke-BuildUI -Context $ctx -Versions $versions
    }.GetNewClosure()

    Invoke-LogSection "Packing client" {
        Invoke-PackClient -Context $ctx -Tools $tools -Versions $versions
    }.GetNewClosure()

    if ($ctx.IsPublicBuild) {
        Invoke-LogSection "Creating sentry release" {
            $sentryVersion = "cfx-{0}" -f $versions.BuildID

            Invoke-SentryCreateRelease -Context $ctx -Version $sentryVersion
        }.GetNewClosure()
    }

    $ctx.finishBuild()
} catch {
    Write-Host $_
    Write-Host "BUILD FAILED"

    Exit 1
}
