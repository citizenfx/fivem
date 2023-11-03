using module .\psm1\cfxVersions.psm1
using module .\psm1\cfxBuildTools.psm1
using module .\psm1\cfxBuildContext.psm1
using module .\psm1\cfxBuildCacheMeta.psm1
using module .\psm1\cfxGitlabSections.psm1
using module .\psm1\cfxSentry.psm1

try {
    $ctx = Get-CfxBuildContext -RequireProductName
    $tools = Get-CfxBuildTools -Context $ctx
    $versions = Get-CfxVersions -ProjectRoot $ctx.ProjectRoot

    if ($ctx.IS_FXSERVER) {
        throw "This script cannot deploy FXServer"
    }

    if (!$env:REFRESH_URL) {
        throw "No REFRESH_URL env var"
    }

    if (!$env:CI_ENVIRONMENT_NAME) {
        throw "No CI_ENVIRONMENT_NAME env var"
    }

    $updateChannelName = $env:CI_ENVIRONMENT_NAME

    Write-Output "Context:", $ctx, "`n"
    Write-Output "Tools:", $tools, "`n"
    Write-Output "Versions:", $versions, "`n"

    $ctx.startBuild()

    Invoke-LogSection ("Deploying {0}" -f $ctx.ProductName) {
        $CacheName = "fivereborn"

        if ($ctx.IS_REDM) {
            $CacheName = "redm"
        }

        $params = @{
            Context = $ctx
            Tools = $tools
            Versions = $versions

            UpdateChannelName = $updateChannelName
            UpdateChannelVersion = $versions.Game.ToString()

            CacheDir = [IO.Path]::Combine($ctx.CachesRoot, "fivereborn")
            CacheName = $CacheName
        }

        Invoke-BuildCacheMeta @params
    }.GetNewClosure()

    # notify services as soon as possible
    Invoke-LogSection "Services notification" {
        $oldSecurityProtocol = [Net.ServicePointManager]::SecurityProtocol
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

        $retriesLeft = 5
        $succeeded = $false

        while ($retriesLeft -ne 0) {
            try {
                Invoke-WebRequest -UseBasicParsing -Uri $env:REFRESH_URL -Method GET | Out-Null

                $succeeded = $true
                break
            }
            catch {
                Write-Host "Notifying services failed:" $_
                Write-Host "Retries left #$retriesLeft"

                $retriesLeft -= 1

                Start-Sleep -Milliseconds 500
            }
        }

        [Net.ServicePointManager]::SecurityProtocol = $oldSecurityProtocol

        if (!$succeeded) {
            Write-Host "Failed to notify services"
            $Context.addBuildWarning("Failed to notify services")
        }
    }.GetNewClosure()

    # deploy FxDK for FiveM as well
    if ($ctx.IS_FIVEM) {
        Invoke-LogSection "Deploying FxDK for FiveM" {
            $params = @{
                Context = $ctx
                Tools = $tools
                Versions = $versions

                UpdateChannelName = $updateChannelName
                UpdateChannelVersion = $versions.SDK.ToString()

                CacheDir = [IO.Path]::Combine($ctx.CachesRoot, "fxdk-five")
                CacheName = "fxdk-five"
            }
    
            Invoke-BuildCacheMeta @params
        }.GetNewClosure()
    }

    if ($Context.IsReleaseBuild) {
        Invoke-LogSection "Creating sentry deploy" {
            Invoke-SentryCreateDeploy -Context $ctx -Versions $versions -Environment $updateChannelName
        }.GetNewClosure()
    }

    $ctx.finishBuild()
} catch {
    Write-Output $_
    Write-Output "DEPLOY FAILED"

    Exit 1
}
