using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxCacheVersions.psm1
using module .\cfxVersions.psm1

function Invoke-BuildSDK {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools,
        [CfxVersions] $Versions
    )

    # FXCode building is broken since Oct 7th 2023 due to deprecation of references-view extension
    # Workaround that by reusing already built FXCode version since we're not updating FXCode itself anymore
    Push-Location $Context.getPathInProject("ext\sdk\resources\sdk-root\fxcode")
        if (!(Test-Path "out-fxdk-pkg")) {
            curl.exe -Lo ".\out-fxdk-pkg.zip" "https://content.cfx.re/mirrors/vendor/fxcode/out-fxdk-pkg.zip"

            & $Tools.tar -C .\ -xf ".\out-fxdk-pkg.zip"

            Remove-Item -Force ".\out-fxdk-pkg.zip"
        }
    Pop-Location

    # Build SDK
    Push-Location $Context.getPathInProject("ext\sdk-build")
        cmd /c .\build.cmd --build-fxcode=$false
        Test-LastExitCode "Failed to build SDK"
    Pop-Location

    # Packing
    $cacheName = "fxdk-five"

    $packRoot = [IO.Path]::Combine($Context.CachesRoot, $cacheName)
    $cachesRoot = $Context.CachesRoot
    $sdkGameRoot = $Context.getPathInProject("ext\sdk\resources\sdk-game")

    # Ensure target dir layout
    Invoke-EnsureDirExists $packRoot\citizen\sdk\sdk-root
    Invoke-EnsureDirExists $packRoot\citizen\sdk\sdk-game

    robocopy $Context.getPathInProject("ext\sdk-build\sdk-root\resource\") $packRoot\citizen\sdk\sdk-root\ /mir /xo /fft /ndl /njh /njs /nc /ns /np

    Copy-Item -Force $sdkGameRoot\fxmanifest.lua $packRoot\citizen\sdk\sdk-game\
    Copy-Item -Force $sdkGameRoot\sdk-client.js  $packRoot\citizen\sdk\sdk-game\
    Copy-Item -Force $sdkGameRoot\sdk-server.js  $packRoot\citizen\sdk\sdk-game\
    
    Write-FxDKCacheVersions -CachesRoot $cachesRoot -CacheName $cacheName -Versions $Versions
}
