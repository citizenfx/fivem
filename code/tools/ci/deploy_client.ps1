using module .\psm1\cfxBuildTools.psm1
using module .\psm1\cfxBuildContext.psm1
using module .\psm1\cfxBuildCacheMeta.psm1
using module .\psm1\cfxCacheVersions.psm1
using module .\psm1\cfxGitlabSections.psm1
using module .\psm1\cfxSentry.psm1

try {
    $ctx = Get-CfxBuildContext -RequireProductName
    $tools = Get-CfxBuildTools -Context $ctx

    if ($ctx.IS_FXSERVER) {
        throw "This script cannot deploy FXServer"
    }

    # if (!$env:REFRESH_URL) {
    #     throw "No REFRESH_URL env var"
    # }

    if (!$env:CI_ENVIRONMENT_NAME) {
        throw "No CI_ENVIRONMENT_NAME env var"
    }

    $updateChannelName = $env:CI_ENVIRONMENT_NAME
    
    $clientCacheName = "fivereborn"
    $clientVersions = Read-ClientCacheVersions -CacheName $clientCacheName -CachesRoot $ctx.CachesRoot

    Write-Output "Context:", $ctx, "`n"
    Write-Output "Tools:", $tools, "`n"
    Write-Output "Client versions:", $clientVersions, "`n"

    $ctx.startBuild()

    Invoke-LogSection ("Deploying {0}" -f $ctx.ProductName) {
        $cacheDir = [IO.Path]::Combine($ctx.CachesRoot, $clientCacheName)
        $cacheName = $clientCacheName

        # Fix up cache name for RedM, it uses the same cache dir as FiveM, but has it's own name
        if ($ctx.IS_REDM) {
            $cacheName = "redm"
        }

        $params = @{
            Context = $ctx
            Tools = $tools
            
            BootstrapVersion = $clientVersions.BootstrapVersion

            UpdateChannelName = $updateChannelName
            UpdateChannelVersion = $clientVersions.UpdateChannelVersion

            CacheDir = $cacheDir
            CacheName = $cacheName
        }

        Invoke-BuildCacheMeta @params
    }.GetNewClosure()

    # notify services as soon as possible
    # Invoke-LogSection "Services notification" {
    #     if ($Context.IsDryRun) {
    #         Write-Output "DRY RUN: Would notify services about new deployment"
    #     } else {
    #         $oldSecurityProtocol = [Net.ServicePointManager]::SecurityProtocol
    #         [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    
    #         $retriesLeft = 5
    #         $succeeded = $false
    
    #         while ($retriesLeft -ne 0) {
    #             try {
    #                 Invoke-WebRequest -UseBasicParsing -Uri $env:REFRESH_URL -Method GET | Out-Null
    
    #                 $succeeded = $true
    #                 break
    #             }
    #             catch {
    #                 Write-Host "Notifying services failed:" $_
    #                 Write-Host "Retries left #$retriesLeft"
    
    #                 $retriesLeft -= 1
    
    #                 Start-Sleep -Milliseconds 500
    #             }
    #         }
    
    #         [Net.ServicePointManager]::SecurityProtocol = $oldSecurityProtocol
    
    #         if (!$succeeded) {
    #             Write-Host "Failed to notify services"
    #             $Context.addBuildWarning("Failed to notify services")
    #         }
    #     }
    # }.GetNewClosure()

    # deploy FxDK for FiveM as well
    # if ($ctx.IS_FIVEM) {
    #     $cacheName = "fxdk-five"
    #     $cacheDir = [IO.Path]::Combine($ctx.CachesRoot, $cacheName)

    #     $fxdkVersions = Read-FxDKCacheVersions -CacheName $cacheName -CachesRoot $ctx.CachesRoot

    #     Write-Output "FxDK versions:", $fxdkVersions, "`n"

    #     Invoke-LogSection "Deploying FxDK for FiveM" {
    #         $params = @{
    #             Context = $ctx
    #             Tools = $tools
            
    #             BootstrapVersion = $clientVersions.BootstrapVersion

    #             UpdateChannelName = $updateChannelName
    #             UpdateChannelVersion = $fxdkVersions.UpdateChannelVersion

    #             CacheDir = $cacheDir
    #             CacheName = $cacheName
    #         }
    
    #         Invoke-BuildCacheMeta @params
    #     }.GetNewClosure()
    # }

    # if ($Context.IsPublicBuild) {
    #     Invoke-LogSection "Creating sentry deploy" {
    #         $params = @{
    #             Context = $ctx
    #             Version = $clientVersions.BuildID
    #             Environment = $updateChannelName
    #         }

    #         Invoke-SentryCreateDeploy @params
    #     }.GetNewClosure()
    # }

    $ctx.finishBuild()
} catch {
    Write-Output $_
    Write-Output "DEPLOY FAILED"

    Exit 1
}
