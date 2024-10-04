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
            cmd /c build.cmd
            Test-LastExitCode "Failed to build CfxUI"

            $Versions.UICommit | Out-File -Encoding ascii -NoNewline $lastBuiltCommitFile

            # delete node_modules to avoid them polluting the build cache
            Remove-Item -Recurse -Force $Context.getPathInProject("ext\cfx-ui\node_modules")
        } else {
            Write-Output "Skipping UI build: existing artifacts are up to date"
        }
    Pop-Location
}