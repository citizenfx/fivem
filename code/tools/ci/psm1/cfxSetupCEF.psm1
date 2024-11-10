using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1

function Invoke-CfxSetupCEF {
    param(
        [CfxBuildTools] $Tools,
        [CfxBuildContext] $Context
    )

    $CefDir = $Context.getPathInProject("vendor\cef")
    $CefCacheDir = $Context.getPathInBuildCache("cef-cache")

    New-Item -ItemType Directory -Force -Path $CefCacheDir | Out-Null

    $CefBuildName = (Get-Content -ErrorAction Stop -Encoding ascii "$CefDir\cef_build_name.txt").Trim()

    $CachedCefBuildNameFilePath = "$CefCacheDir\current-cef-build-name.txt"
    $CachedCefBuildName = Get-Content -ErrorAction Ignore -Encoding ascii $CachedCefBuildNameFilePath
    $CachedCefBuildDir = "$CefCacheDir\cef"

    if (($CachedCefBuildName -ne $CefBuildName) -or !(Test-Path $CachedCefBuildDir)) {
        Remove-Item -Force -Recurse -ErrorAction Ignore $CachedCefBuildDir
        
        $CachedCefBuildArchive = "$CefCacheDir\$CefBuildName.zip"

        curl.exe -Lo $CachedCefBuildArchive "https://cdn.vmp.ir/build/cef/$CefBuildName.zip"

        # unpack cef
        $tempDir = "$CefCacheDir\$([System.Guid]::NewGuid())"
        New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

        & $Tools.tar -C $tempDir -xf $CachedCefBuildArchive

        Move-Item -Force -Path "$tempDir\$CefBuildName" -Destination $CachedCefBuildDir

        # cleanup
        Remove-Item -Force -Recurse $tempDir
        Remove-Item -Force $CachedCefBuildArchive

        $CefBuildName | Out-File -Encoding ascii -NoNewline $CachedCefBuildNameFilePath
    }

    Copy-Item -Force -Recurse -Path $CachedCefBuildDir\* -Destination $CefDir\
}