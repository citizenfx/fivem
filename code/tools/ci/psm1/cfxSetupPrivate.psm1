using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxVersions.psm1
using module .\cfxSetupSubmodules.psm1

function Invoke-CfxSetupPrivate {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions
    )

    if (!$Context.PrivateUri -or !$Context.ClosedUri) {
        $Context.addBuildWarning("Private or closed URI not set, skipping setup")
        return
    }

    # We'll be passing that to closures so we need it to be an object with an array inside to allow closures to modify it
    $privateRepoPaths = @{
        "Arr" = @()
    }

    Invoke-LogSection "Setting up private" {
        $privateBranch = $Context.GitBranchName

        if ($env:CFX_OVERRIDE_PRIVATE_BRANCH) {
            $privateBranch = $env:CFX_OVERRIDE_PRIVATE_BRANCH
            $Context.addBuildWarning("Overriding private branch to $privateBranch")
        }

        $repo = @{
            "Context" = $Context
            "Versions" = $Versions
            "Name" = "private"
            "URI" = $Context.PrivateUri
            "Root" = $Context.PrivateRoot
            "_Branch" = $privateBranch
        }
        
        Invoke-SetupRepo @repo

        $privateRepoPaths.Arr += $Context.PrivateRoot
    }.GetNewClosure()

    if (!$Context.IS_FXSERVER) {
        Invoke-LogSection "Setting up closed" {
            $closedBranch = $Context.GitBranchName

            if ($env:CFX_OVERRIDE_CLOSED_BRANCH) {
                $Context.addBuildWarning("Overriding closed branch to $closedBranch")
                $closedBranch = $env:CFX_OVERRIDE_CLOSED_BRANCH
            }

            $repo = @{
                "Context"  = $Context
                "Versions" = $Versions
                "Name"     = "closed"
                "URI"      = $Context.ClosedUri
                "Root"     = $Context.ClosedRoot
                "_Branch"  = $closedBranch
            }

            Invoke-SetupRepo @repo

            $privateRepoPaths.Arr += $Context.ClosedRoot
        }.GetNewClosure()
    }

    # Generate privates config with paths relative to the code root
    Push-Location $Context.CodeRoot
        $privatesConfig = $privateRepoPaths.Arr | ForEach-Object {
            $relativePath = (Resolve-Path -Relative $_) -replace '\\','/'

            "private_repo '$relativePath/'"
        }
    Pop-Location

    # Write the privates config
    $privatesConfigPath = [IO.Path]::Combine($Context.CodeRoot, "privates_config.lua")
    $privatesConfig | Out-File -Encoding ascii $privatesConfigPath
}

function Invoke-SetupRepo {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions,
        [string] $Name,
        [string] $URI,
        [string] $Root,
        [string] $_Branch
    )

    $Branch = $_Branch

    # Check if we have matching branch, fallback to master otherwise
    # See https://git-scm.com/docs/git-ls-remote/2.6.7#Documentation/git-ls-remote.txt---exit-code
    # for the exit code `2` meaning that the branch does not exist
    git ls-remote --exit-code --heads $URI refs/heads/$Branch | Out-Null
    if ($LASTEXITCODE -eq 2) {
        $Context.addBuildWarning("Branch $Branch not found in $Name, falling back to master")
        $Branch = "master"
    }

    if (Test-Path "$Root\.git") {
        Push-Location $Root
            git remote set-url origin $URI | Out-Null
            Test-LastExitCode "Failed to set origin url of $Name"

            git fetch origin | Out-Null
            Test-LastExitCode "Failed to fetch $Name"

            git reset --hard origin/$Branch | Out-Null
            Test-LastExitCode "Failed to reset $Name"
        Pop-Location
    } else {
        $parentPath = [IO.Path]::GetDirectoryName($Root)
        $cloneToPath = [IO.Path]::GetFileName($Root)

        Remove-Item -Force -Recurse -ErrorAction Ignore $Root | Out-Null

        # Ensure parent directory exists
        New-Item -ItemType Directory -Force $parentPath | Out-Null

        Push-Location $parentPath
            git clone --recursive -b $Branch $URI $cloneToPath | Out-Null
            Test-LastExitCode "Failed to clone $Name"
        Pop-Location
    }

    # work on submodules, if any
    Invoke-SetupSubmodules -Root $Root

    # adjust game version
    Push-Location $Root
        $Versions.Game += (git rev-list HEAD | measure-object).Count * 10
    Pop-Location
}
