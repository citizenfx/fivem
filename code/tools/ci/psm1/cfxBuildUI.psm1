using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxVersions.psm1

function Invoke-BuildUI {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions
    )

    Push-Location $Context.getPathInProject("ext\ui-build")
        $lastBuiltCommitFile = "data\.commit"

        $havePreviouslyBuiltArtifacts = (Test-Path "data.zip") -and (Test-Path "data_big.zip")
        $haveNewerCommit = $Versions.UICommit -ne (Get-Content -ErrorAction Ignore $lastBuiltCommitFile)

        if (!$havePreviouslyBuiltArtifacts -or $haveNewerCommit) {
            $cfxUINodeModulesCacheParams = @{
                Context = $Context
                CacheName = "cfx-ui-node_modules"
                Path = $Context.getPathInProject("ext\cfx-ui\node_modules")
            }

            # restore node_modules from cache
            Invoke-RestoreFromBuildCache @cfxUINodeModulesCacheParams

            cmd /c build.cmd
            Test-LastExitCode "Failed to build CfxUI"

            $Versions.UICommit | Out-File -Encoding ascii -NoNewline $lastBuiltCommitFile

            # store node_modules in cache
            Invoke-SaveInBuildCache @cfxUINodeModulesCacheParams
        } else {
            Write-Output "Skipping UI build: existing artifacts are up to date"
        }
    Pop-Location
}