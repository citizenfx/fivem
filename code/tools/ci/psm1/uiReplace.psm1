using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxVersions.psm1

function Invoke-UI {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions
    )

    if (!$Context.UIUri) {
        return
    }

    Invoke-LogSection "Setting up UI" {
        $uiBranch = 'main'
        $UIUri = $Context.UIUri


        # Check if we have matching branch, fallback to master otherwise
        git ls-remote --exit-code --heads $UIUri refs/heads/$uiBranch | Out-Null
        if ($LASTEXITCODE -eq 2) {
            $uiBranch = "master"
        }

        if (Test-Path "$($Context.UIRoot)\.git") {
            Push-Location $Context.UIRoot
                git remote set-url origin $UIUri | Out-Null
                Test-LastExitCode "Failed to set origin url for ui"

                git fetch origin | Out-Null
                Test-LastExitCode "Failed to fetch ui"

                git reset --hard origin/$uiBranch | Out-Null
                Test-LastExitCode "Failed to reset ui"
            Pop-Location
        } else {
            $parentPath = [IO.Path]::GetDirectoryName($Context.UIRoot)
            $cloneToPath = [IO.Path]::GetFileName($Context.UIRoot)

            Remove-Item -Force -Recurse -ErrorAction Ignore $Context.UIRoot | Out-Null

            # Ensure parent directory exists
            New-Item -ItemType Directory -Force $parentPath | Out-Null

            Push-Location $parentPath
                git clone -b $uiBranch $UIUri $cloneToPath | Out-Null
                Test-LastExitCode "Failed to clone ui"
            Pop-Location
        }

        Push-Location $Context.UIRoot
            $Versions.UICommit = (git rev-list -1 HEAD)
            $Versions.UI = ((git rev-list $Versions.UICommit | measure-object).Count * 10) + 1100000
        Pop-Location

        Push-Location $Context.ProjectRoot
            Remove-Item -Path "ext/cfx-ui/*" -Recurse -Force
            Copy-Item -Path "$($Context.UIRoot)\*" -Destination "ext/cfx-ui" -Recurse
        Pop-Location

    }.GetNewClosure()
}
