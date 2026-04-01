using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxGitlabSections.psm1

function Invoke-SentryCreateRelease {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools,
        [string] $Version,
        [string] $ProjectName
    )

    if (!$ProjectName) {
        $ProjectName = $Context.SentryProjectName
    }

    if (!$env:SENTRY_AUTH_TOKEN) {
        $Context.addBuildWarning("Skipping creating sentry release as SENTRY_AUTH_TOKEN env var is missing")
        return
    }

	if (!$env:CFX_SENTRY_FIVEM_REPOSITORY) {
		$Context.addBuildWarning("Skipping creating sentry release as CFX_SENTRY_FIVEM_REPOSITORY env var is missing")
		return
	}

    $sentryCLI = $Tools.getSentryCLI()

    if ($Context.IsDryRun) {
        Write-Output "DRY RUN: Would create sentry release $Version for project $ProjectName"
    } else {
        & $sentryCLI @(
            "releases", "new", $Version
            "--org", $Context.SentryOrgName
            "--project", $ProjectName
            "--finalize"
        )
        if ($LASTEXITCODE -ne 0) {
            $Context.addBuildWarning("Failed to create release in sentry")
            return
        }

        # Temporarily disabled until GitHub integration in Sentry is enabled again
        <#& $sentryCLI @(
            "releases", "set-commits", $Version
            "--org", $Context.SentryOrgName
            "--project", $ProjectName
            "--commit", "$($env:CFX_SENTRY_FIVEM_REPOSITORY)@$($Context.GitCommitSHA)"
        )
        if ($LASTEXITCODE -ne 0) {
            $Context.addBuildWarning("Failed to set commits for sentry release")
        }#>
    }
}

function Invoke-SentryCreateDeploy {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools,
        [string] $Environment,
        [string] $Version,
        [string] $ProjectName
    )

    if (!$ProjectName) {
        $ProjectName = $Context.SentryProjectName
    }

    if (!$env:SENTRY_AUTH_TOKEN) {
        $Context.addBuildWarning("Skipping creating sentry deploy as SENTRY_AUTH_TOKEN env var is missing")
        return
    }

    $sentryCLI = $Tools.getSentryCLI()

    if ($Context.IsDryRun) {
        Write-Output "DRY RUN: Would create sentry deploy for $Version in $Environment for project $ProjectName"
    } else {
        $Environment = ConvertTo-Slug $Environment

        & $sentryCLI @(
            "releases", "deploys", "new"
            "--release", $Version
            "--org", $Context.SentryOrgName
            "--project", $ProjectName
            "-e", $Environment
        )
        if ($LASTEXITCODE -ne 0) {
            $Context.addBuildWarning("Failed to create deploy in sentry")
        }
    }
}

function Invoke-SentryUploadDebugFiles {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools,
        [string] $Path,
        [string] $ProjectName
    )

    if (!$ProjectName) {
        $ProjectName = $Context.SentryProjectName
    }

    if (!$env:SENTRY_AUTH_TOKEN) {
        $Context.addBuildWarning("Skipping uploading debug files to Sentry as SENTRY_AUTH_TOKEN env var is missing")
        return
    }

    if (!(Test-Path $Path)) {
        throw "Specified path for debug files does not exist"
    }

    $sentryCLI = $Tools.getSentryCLI()

    if ($Context.IsDryRun) {
        Write-Output "DRY RUN: Would upload debug files to Sentry from $Path for project $ProjectName"
    } else {
        & $sentryCLI @(
            "debug-files"
            "upload"
            "-o", $Context.SentryOrgName
            "-p", $ProjectName
            "--include-sources"
            "$Path\"
        )

        if ($LASTEXITCODE -ne 0) {
            $Context.addBuildWarning("Failed to upload debug information files to sentry")
        }
    }
}
