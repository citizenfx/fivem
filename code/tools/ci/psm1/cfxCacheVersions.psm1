using module .\cfxVersions.psm1

class CfxClientCacheVersions {
    [string] $BuildID
    [string] $BootstrapVersion
    [string] $UpdateChannelVersion
}

class CfxFxDKCacheVersions {
    [string] $updateChannelVersion
}

function Write-ClientCacheVersions {
    param(
        [string] $CacheName,
        [string] $CachesRoot,
        [CfxVersions] $Versions
    )

    $cachesXML = @(
        '<Caches>'
        '  <Cache ID="{0}" Version="{1}" />' -f $CacheName, $Versions.Game
        '</Caches>'
    ) -join "`n"
    $cachesXML | Out-File -Encoding ascii $CachesRoot\caches.xml

    $manifest = ConvertTo-Json @{
        buildId = $Versions.BuildID
        bootstrapVersion = $Versions.Launcher.ToString()
        updateChannelVersion = $Versions.Game.ToString()
    }
    $manifest | Out-File -Encoding ascii $CachesRoot\$CacheName.json
}

function Read-ClientCacheVersions {
    param(
        [string] $CacheName,
        [string] $CachesRoot
    )

    $manifestPath = "$CachesRoot\$CacheName.json"
    if (!(Test-Path $manifestPath)) {
        throw "Client cache versions manifest file not found"
    }

    $manifest = Get-Content -Encoding ascii $manifestPath | ConvertFrom-Json
    if (!$manifest) {
        throw "Invalid or empty client cache versions manifest"
    }

    if (!$manifest.buildId) {
        throw "Invalid client cache versions manifest, missing 'buildId' field"
    }
    if (!$manifest.bootstrapVersion) {
        throw "Invalid client cache versions manifest, missing 'bootstrapVersion' field"
    }
    if (!$manifest.updateChannelVersion) {
        throw "Invalid client cache versions manifest, missing 'updateChannelVersion' field"
    }

    $clientVersions = [CfxClientCacheVersions]::new()
    $clientVersions.BuildID = $manifest.buildId
    $clientVersions.BootstrapVersion = $manifest.bootstrapVersion
    $clientVersions.UpdateChannelVersion = $manifest.updateChannelVersion

    return $clientVersions
}

function Write-FxDKCacheVersions {
    param(
        [string] $CacheName,
        [string] $CachesRoot,
        [CfxVersions] $Versions
    )

    $cachesXML = @(
        '<Caches>'
        '  <Cache ID="{0}" Version="{1}" />' -f $CacheName, $Versions.SDK
        '</Caches>'
    ) -join "`n"
    $cachesXML | Out-File -Encoding ascii $CachesRoot\caches_sdk.xml

    $manifest = ConvertTo-Json @{
        updateChannelVersion = $Versions.SDK.ToString()
    }
    $manifest | Out-File -Encoding ascii $CachesRoot\$CacheName.json
}

function Read-FxDKCacheVersions {
    param(
        [string] $CacheName,
        [string] $CachesRoot
    )

    $manifestPath = "$CachesRoot\$CacheName.json"
    if (!(Test-Path $manifestPath)) {
        throw "FxDK cache versions manifest file not found"
    }

    $manifest = Get-Content -Encoding ascii $manifestPath | ConvertFrom-Json
    if (!$manifest) {
        throw "Invalid or empty FxDK cache versions manifest"
    }

    if (!$manifest.updateChannelVersion) {
        throw "Invalid FxDK cache versions manifest, missing 'updateChannelVersion' field"
    }

    $fxdkVersions = [CfxFxDKCacheVersions]::new()
    $fxdkVersions.UpdateChannelVersion = $manifest.updateChannelVersion

    return $fxdkVersions
}