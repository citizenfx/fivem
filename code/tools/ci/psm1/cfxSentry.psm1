using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxVersions.psm1

function Invoke-SentryCreateRelease {
    param(
        [CfxBuildContext] $Context,
        [string] $Version
    )

    if (!$env:SENTRY_TOKEN) {
        $Context.addBuildWarning("Skipping creating sentry release as SENTRY_TOKEN env var is missing")
        return
    }

    $sentryOrgName = $Context.SentryOrgName

    $request = @{
        Uri         = "https://sentry.fivem.net/api/0/organizations/$sentryOrgName/releases/"
        Method      = "Post"
        Headers     = @{
            Authorization = "Bearer $env:SENTRY_TOKEN"
        }
        ContentType = "application/json"
        Body        = ConvertTo-Json @{
            version  = $Version
            refs     = @(
                @{
                    repository = 'citizenfx/fivem'
                    commit     = $Context.GitCommitSHA
                }
            )
            projects = @(
                $Context.SentryProjectName
            )
        }
    }

    try {
        Invoke-RestMethod @request
    }
    catch {
        Write-Host $_
        $Context.addBuildWarning("Failed to create release in sentry")
    }
}

function Invoke-SentryCreateDeploy {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions,
        [string] $Environment
    )

    if (!$env:SENTRY_TOKEN) {
        $Context.addBuildWarning("Skipping creating sentry deploy as SENTRY_TOKEN env var is missing")
        return
    }

    $sentryVersion = $Versions.BuildID
    $sentryOrgName = $Context.SentryOrgName

    $request = @{
        Uri         = "https://sentry.fivem.net/api/0/organizations/$sentryOrgName/releases/$sentryVersion/deploys/"
        Method      = "Post"
        Headers     = @{
            Authorization = "Bearer $env:SENTRY_TOKEN"
        }
        ContentType = "application/json"
        Body        = ConvertTo-Json @{
            environment = $Environment
            projects    = @($Context.SentryProjectName)
        }
    }

    try {
        Invoke-RestMethod @request
    }
    catch {
        Write-Host $_
        $Context.addBuildWarning("Failed to create release in sentry")
    }
}

function Invoke-SentryUploadDif {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools,
        [string] $Path
    )

    if (!$env:SENTRY_AUTH_TOKEN) {
        $Context.addBuildWarning("Skipping creating sentry deploy as SENTRY_AUTH_TOKEN env var is missing")
        return
    }

    if (!(Test-Path $Path)) {
        throw "Specified path for upload-dif does not exist"
    }

    $sentryCLI = $Tools.getSentryCLI()

    & $sentryCLI @(
        "upload-dif"
        "--org", $Context.SentryOrgName
        "--project", $Context.SentryProjectName
        "$Path\"
    )
    if ($LASTEXITCODE -ne 0) {
        $Context.addBuildWarning("Failed to upload debug information files to sentry")
    }
}