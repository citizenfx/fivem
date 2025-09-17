using module .\cfxBuildContext.psm1

function Invoke-CfxSetupBuildToolkit {
    param(
        [CfxBuildContext] $Context
    )

    if (!$Context.ToolkitUri) {
        return
    }

    Invoke-LogSection "Setting up the build toolkit" {
        $toolkitBranch = $Context.GitBranchName
        $toolkitUri = $Context.ToolkitUri

        if ($env:CFX_OVERRIDE_TOOLKIT_BRANCH) {
            $toolkitBranch = $env:CFX_OVERRIDE_TOOLKIT_BRANCH
        }

        # Check if we have matching branch, fallback to master otherwise
        git ls-remote --exit-code --heads $toolkitUri refs/heads/$toolkitBranch | Out-Null
        if ($LASTEXITCODE -eq 2) {
            $toolkitBranch = "master"
        }

        # Get/Update the toolkit
        if (Test-Path "$($Context.ToolkitRoot)\.git") {
            Push-Location $Context.ToolkitRoot
                git remote set-url origin $toolkitUri | Out-Null
                Test-LastExitCode "Failed to set origin url for the toolkit"

                git fetch origin | Out-Null
                Test-LastExitCode "Failed to fetch the toolkit"

                git reset --hard origin/$toolkitBranch | Out-Null
                Test-LastExitCode "Failed to reset the toolkit"
            Pop-Location
        } else {
            $parentPath = [IO.Path]::GetDirectoryName($Context.ToolkitRoot)
            $cloneToPath = [IO.Path]::GetFileName($Context.ToolkitRoot)

            Remove-Item -Force -Recurse -ErrorAction Ignore $Context.ToolkitRoot | Out-Null

            # Ensure parent directory exists
            New-Item -ItemType Directory -Force $parentPath | Out-Null

            Push-Location $parentPath
                git clone --filter=tree:0 -b $toolkitBranch $toolkitUri $cloneToPath | Out-Null
                Test-LastExitCode "Failed to clone the toolkit"
            Pop-Location
        }

        # Setup toolkit env vars
        $tookitSetupEnvPath = "$($Context.ToolkitRoot)\setupEnv.ps1"
        Test-PathExists $tookitSetupEnvPath "Toolkit env setup script not found: $tookitSetupEnvPath"

        & $tookitSetupEnvPath | Out-Null
        Test-LastExitCode "Failed to setup the toolkit environment"
    }.GetNewClosure()
}
