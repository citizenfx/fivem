using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxVersions.psm1

function Invoke-CfxSetupPrivate {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions
    )

    if (!$Context.PrivateUri) {
        return
    }

    Invoke-LogSection "Setting up private" {
        $privateBranch = $Context.GitBranchName
        $privateUri = $Context.PrivateUri

        # Check if we have matching branch, fallback to master otherwise
        git ls-remote --exit-code --heads $privateUri refs/heads/$privateBranch | Out-Null
        if ($LASTEXITCODE -eq 2) {
            $privateBranch = "master"
        }

        if (Test-Path "$($Context.PrivateRoot)\.git") {
            Push-Location $Context.PrivateRoot
                git fetch origin | Out-Null
                Test-LastExitCode "Failed to fetch private"

                git reset --hard origin/$privateBranch | Out-Null
                Test-LastExitCode "Failed to reset private"
            Pop-Location
        } else {
            $parentPath = [IO.Path]::GetDirectoryName($Context.PrivateRoot)
            $cloneToPath = [IO.Path]::GetFileName($Context.PrivateRoot)

            Remove-Item -Force -Recurse -ErrorAction Ignore $Context.PrivateRoot | Out-Null

            Push-Location $parentPath
                git clone -b $privateBranch $privateUri $cloneToPath | Out-Null
                Test-LastExitCode "Failed to clone private"
            Pop-Location
        }

        # account for a private version
        Push-Location $Context.PrivateRoot
            $Versions.Game += (git rev-list HEAD | measure-object).Count * 10
        Pop-Location

        Push-Location $Context.CodeRoot
            $relativePathToPrivateFromCode = (Resolve-Path -Relative $Context.PrivateRoot) -replace '\\','/'

            "private_repo '$relativePathToPrivateFromCode/'" | Out-File -Encoding ascii privates_config.lua
        Pop-Location
    }.GetNewClosure()
}