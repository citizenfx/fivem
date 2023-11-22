using module .\cfxBuildContext.psm1

class CfxVersions {
    [int] $Game

    [string] $BuildID

    [string] $CEFName

    [int] $Launcher
    [string] $LauncherCommit

    [int] $SDK
    [string] $SDKCommit

    [int] $UI
    [string] $UICommit

    [int] $SystemResources
    [string] $SystemResourcesCommit
}

function Get-CfxVersions {
    param(
        [CfxBuildContext] $Context
    )

    $versions = [CfxVersions]::new()

    if ($env:CI_PIPELINE_ID) {
        $versions.BuildID = $env:CI_PIPELINE_ID
    }

    Push-Location $Context.ProjectRoot
        # for builds with private this will also be adjusted accordingly, see cfxSetupPrivate.psm1
        $versions.Game = ((git rev-list HEAD | measure-object).Count * 10) + 1100000

        $versions.CEFName = (Get-Content -Encoding ascii vendor\cef\cef_build_name.txt).Trim()

        $LauncherPaths = @(
            "code/premake5.lua"
            "code/shared/"
            "code/tools/dbg/"
            "code/client/launcher/"
            "code/client/shared/"
            "vendor/breakpad/"
            "vendor/tinyxml2/"
            "vendor/xz/"
            "vendor/curl/"
            "vendor/cpr/"
            "vendor/minizip/"
        )
        $versions.LauncherCommit = (git rev-list -1 HEAD $LauncherPaths)
        $versions.Launcher = ((git rev-list $versions.LauncherCommit | measure-object).Count * 10) + 1100000

        $SDKPaths = @(
            "ext/sdk-build/"
            "ext/sdk/"
            "code/tools/ci/build_sdk.ps1"
        )
        $versions.SDKCommit = (git rev-list -1 HEAD $SDKPaths)
        $versions.SDK = ((git rev-list $versions.SDKCommit | measure-object).Count * 10) + 1100000

        $UIPaths = @(
            "ext/cfx-ui/"
            "ext/ui-build/"
        )
        $versions.UICommit = (git rev-list -1 HEAD $UIPaths)
        $versions.UI = ((git rev-list $versions.UICommit | measure-object).Count * 10) + 1100000

        $SystemResourcesPaths = @(
            "ext/system-resources/"
        )
        $versions.SystemResourcesCommit = (git rev-list -1 HEAD $SystemResourcesPaths)
        $versions.SystemResources = ((git rev-list $versions.SystemResourcesCommit | measure-object).Count * 10) + 1100000
    Pop-Location

    return $versions
}