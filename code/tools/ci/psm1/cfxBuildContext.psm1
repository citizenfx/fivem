class CfxBuildContext {
    [string] $ProjectRoot

    [boolean] $IsDryRun
    [boolean] $IsPublicBuild
    [boolean] $IsRetailBuild
    [boolean] $IsFeatureBranchBuild

    [string] $ProductName
    [string] $ProductExeName

    [bool] $IS_FIVEM = $false
    [bool] $IS_REDM = $false
    [bool] $IS_FXSERVER = $false

    [string] $GitBranchName
    [string] $GitTag = ""
    [string] $GitCommitSHA

    [string] $CodeRoot
    [string] $BuildCacheRoot
    [string] $CachesRoot

    [string] $PremakeBinDir
    [string] $PremakeBuildDir
    [string] $PremakeGameName
    [string] $PremakeVSVersion

    [string] $VSDir
    [string] $MSBuildOutput
    [string] $MSBuildSolution

    [string] $PrivateRoot = ""
    [string] $PrivateUri = ""

    [string] $ToolkitRoot = ""
    [string] $ToolkitUri = ""

    [string] $SentryOrgName = "citizenfx"
    [string] $SentryProjectName

    [string] getPathInProject([string] $relativePath) {
        return [IO.Path]::Combine($this.ProjectRoot, $relativePath)
    }

    [string] getPathInBuildCache([string] $relativePath) {
        return [IO.Path]::Combine($this.BuildCacheRoot, $relativePath)
    }

    [datetime] hidden $_startedAt
    [void] startBuild() {
        $this._startedAt = Get-Date
    }

    [string[]] hidden $_buildWarnings = @()
    [void] addBuildWarning([string] $Message) {
        $this._buildWarnings += $Message
    }

    [void] finishBuild() {
        $elapsedTime = "{0:mm}m {0:ss}s" -f ((Get-Date) - $this._startedAt)

        $esc = $([char]27)
        $green = "$esc[32m"
        $yellow = "$esc[33m"
        $reset = "$esc[0m"

        $warningsAmount = $this._buildWarnings.Length

        if ($warningsAmount -eq 0) {
            Write-Host "${green}BUILD SUCCEEDED in $elapsedTime ${reset}"
        }
        else {
            Write-Host "${yellow}BUILD COMPLETED in $elapsedTime ${reset}"

            Write-Host $yellow
            Write-Host "Warnings: $warningsAmount"

            [int] $index = 0

            foreach ($warning in $this._buildWarnings) {
                $index += 1

                Write-Host "#${index}: $warning"
            }

            Write-Host $reset
        }
    }
}

function Get-CfxBuildContext {
    param(
        [switch] $RequireProductName
    )

    if (!$env:CI) {
        throw "These scripts are only meant to run by CI/CD"
    }

    $ctx = [CfxBuildContext]::new()

    $ctx.GitBranchName = $env:CI_COMMIT_REF_NAME
    $ctx.GitCommitSHA = $env:CI_COMMIT_SHA

    $ctx.ProjectRoot = $env:CI_PROJECT_DIR -replace '/', '\'

    $ctx.IsDryRun = $env:CFX_DRY_RUN -eq "true"
    $ctx.IsRetailBuild = $env:CFX_RETAIL_BUILD -eq "true"
    $ctx.IsFeatureBranchBuild = $env:CFX_FEATURE_BRANCH_BUILD -eq "true"

    $ctx.IsPublicBuild = $ctx.IsRetailBuild -or $ctx.IsFeatureBranchBuild

    # Perform basic checks
    if (!(Test-Path $ctx.ProjectRoot)) {
        throw "CI provided the project root that does not exist"
    }
    if (!$ctx.GitBranchName) {
        throw "CI did not provide the branch name"
    }

    $ctx.BuildCacheRoot = [IO.Path]::GetFullPath($ctx.getPathInProject(".build-cache"))

    $ctx.CodeRoot = $ctx.getPathInProject("code")
    $ctx.CachesRoot = $ctx.getPathInProject("caches")

    # Trailing backslash is required for premake
    $ctx.PremakeBinDir = $ctx.getPathInBuildCache("bin\")
    $ctx.PremakeBuildDir = $ctx.getPathInBuildCache("build\")

    # Figure out private
    if ($env:FIVEM_PRIVATE_URI) {
        $ctx.PrivateRoot = $ctx.getPathInBuildCache("fivem-private")
        $ctx.PrivateUri = $env:FIVEM_PRIVATE_URI
    }
    elseif ($ctx.IsPublicBuild) {
        throw "Public build requires FIVEM_PRIVATE_URI env var to be defined"
    }

    # Figure out toolkit
    if ($env:CFX_BUILD_TOOLKIT_URI) {
        $ctx.ToolkitRoot = $ctx.getPathInBuildCache("cfx-build-toolkit")
        $ctx.ToolkitUri = $env:CFX_BUILD_TOOLKIT_URI
    }
    elseif ($ctx.IsPublicBuild) {
        throw "Public build requires CFX_BUILD_TOOLKIT_URI env var to be defined"
    }

    $premakeDirSubpath = ""

    switch ($env:CFX_PRODUCT_NAME) {
        "fivem" {
            $ctx.IS_FIVEM = $true

            $ctx.ProductName = "fivem"
            $ctx.ProductExeName = "FiveM.exe"
            $ctx.PremakeGameName = "five"
            $ctx.SentryProjectName = Get-EnvOrDefault $env:CFX_SENTRY_PROJECT_NAME_FIVEM "fivem-client-1604"

            break
        }
        "redm" {
            $ctx.IS_REDM = $true

            $ctx.ProductName = "redm"
            $ctx.ProductExeName = "CitiLaunch.exe"
            $ctx.PremakeGameName = "rdr3"
            $ctx.SentryProjectName = Get-EnvOrDefault $env:CFX_SENTRY_PROJECT_NAME_REDM "redm"

            break
        }
        "fxserver" {
            $ctx.IS_FXSERVER = $true

            $ctx.ProductName = "fxserver"
            $ctx.ProductExeName = "FXServer.exe"
            $ctx.PremakeGameName = "server"
            $ctx.SentryProjectName = Get-EnvOrDefault $env:CFX_SENTRY_PROJECT_NAME_FXSERVER "fxserver"

            $premakeDirSubpath = "windows\"

            break
        }
        Default {
            if ($RequireProductName) {
                throw "Unknown or missing CFX_PRODUCT_NAME: $env:CFX_PRODUCT_NAME, expected one of the following: fivem, redm, fxserver"
            }
        }
    }

    $ctx.MSBuildOutput = [IO.Path]::Combine($ctx.PremakeBinDir, $ctx.PremakeGameName, $premakeDirSubpath + "release")
    $ctx.MSBuildSolution = [IO.Path]::Combine($ctx.PremakeBuildDir, $ctx.PremakeGameName, $premakeDirSubpath + "CitizenMP.sln")

    return $ctx
}

function Get-EnvOrDefault {
    param(
        [string] $Value,
        [string] $Default
    )

    if ($Value) {
        return $Value
    }

    return $Default
}
