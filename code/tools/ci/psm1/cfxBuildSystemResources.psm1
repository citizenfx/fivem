using module .\cfxBuildContext.psm1
using module .\cfxVersions.psm1

function Invoke-BuildSystemResources {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions
    )

    Push-Location $Context.getPathInProject("ext\system-resources")
        $lastBuiltCommitFile = ".commit"

        $haveNewerCommit = $Versions.SystemResourcesCommit -ne (Get-Content -ErrorAction Ignore $lastBuiltCommitFile)

        if ($haveNewerCommit) {
            cmd /c build.cmd
            Test-LastExitCode "Failed to build system resources"

            $Versions.SystemResourcesCommit | Out-File -Encoding ascii -NoNewline $lastBuiltCommitFile
        } else {
            Write-Output "Skipping system resources build: existing artifacts up to date"
        }
    Pop-Location
}