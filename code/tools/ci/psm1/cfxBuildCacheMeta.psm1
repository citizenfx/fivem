using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1

function Invoke-BuildCacheMeta {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools,

        [string] $BootstrapVersion,

        [string] $UpdateChannelName,
        [string] $UpdateChannelVersion,

        [string] $CacheDir,
        [string] $CacheName
    )

    $bcm = $Tools.getBCM()

    if (!(Test-Path $CacheDir)) {
        throw "Cache directory does not exist: $CacheDir"
    }

    # cleanup legacy file, if exists
    if (Test-Path "$CacheDir\info.xml") {
        Remove-Item -Force -ErrorAction Ignore "$CacheDir\info.xml"
    }

    # setup correct PATH env for BCM as it expects xz.exe to be present there
    $env:PATH += ";" + ([IO.Path]::GetDirectoryName($Tools.xz))

    $bcmParams = @(
        "-s3-name",     $env:CACHE_S3_BUCKET
        "-s3-endpoint", $env:CACHE_S3_ENDPOINT
        "-s3-key",      $env:CACHE_S3_KEY
        "-s3-key-id",   $env:CACHE_S3_KEY_ID

        "-source",     $CacheDir
        "-cache-name", $CacheName

        "-branch-name",    $UpdateChannelName
        "-branch-version", $UpdateChannelVersion

        "-bootstrap-executable", "CitizenFX.exe"
        "-bootstrap-version",    $BootstrapVersion
    )

    if ($Context.IsDryRun) {
        Write-Output "DRY RUN: Would invoke Build Cache Meta with:", $bcmParams, "`n"
    } else {
        & $bcm @bcmParams
        Test-LastExitCode "Failed to build cache meta"
    }
}