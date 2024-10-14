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

        # Temporary Game version fixup to avoid potential version collisions between branches
        # TODO(CFX-699): transition to a string-based version for Game
        #
        # For beta and production branches:
        #  Take advantage of that we multiply commits count by 10, which leaves us with the 0-9 range that we can use to make sure versions do not collide
        #
        # For other branches:
        #   We cannot account for all of them here to fit within the 0-9 range, so instead,
        #   grow up by 1 billion and try to minimize collisions by adding first 7 chars of commit hash as a number
        #
        #   We multiply commits count by 10 and start from 1100000,
        #   so the max amount of commits is 87.794.819 = (INT32_MAX - 1.000.000.000 - 0xFFffFFf - 1.100.000) / 10,
        #   before we hit the INT32_MAX limit, should be plenty for now
        switch ($Context.GitBranchName) {
            "master" {
                # No need to touch version for master/canary
                break
            }
            "beta" {
                $versions.Game += 1
                break
            }
            "production" {
                $versions.Game += 9
                break
            }
            Default {
                $versions.Game += 1000000000;
                $versions.Game += [uint32]("0x" + (git rev-list -1 HEAD).Substring(0, 7)) # max 268435455 0xFFFFFFf
            }
        }

        $versions.CEFName = (Get-Content -Encoding ascii vendor\cef\cef_build_name.txt).Trim()

        $LauncherPaths = @(
            "code/premake5.lua"
            "code/premake5_builds.lua"
            "code/premake5_defines.lua"
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
            "ext/system-resources"
        )
        $versions.SystemResourcesCommit = (git rev-list -1 HEAD $SystemResourcesPaths)
        $versions.SystemResources = ((git rev-list $versions.SystemResourcesCommit | measure-object).Count * 10) + 1100000
    Pop-Location

    return $versions
}
