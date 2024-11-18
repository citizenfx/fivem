using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxCacheVersions.psm1
using module .\cfxVersions.psm1
using module .\doSomething.ps1

function Invoke-PackClient {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools,
        [CfxVersions] $Versions
    )

    $cacheName = "fivereborn"

    $binRoot = $Context.MSBuildOutput
    $packRoot = [IO.Path]::Combine($Context.CachesRoot, $cacheName)
    $cachesRoot = $Context.CachesRoot
    $projectRoot = $Context.ProjectRoot

    # Ensure we always have clean packed client by removing old caches root
    Remove-Item -Force -Recurse -ErrorAction ignore $cachesRoot

    # Create needed caches root folder structure
    Invoke-EnsureDirExists $packRoot\bin\cef
    Invoke-EnsureDirExists $packRoot\citizen

    # Adding CEF
    Copy-Item -Force -Recurse $projectRoot\vendor\cef\Release\*.dll               $packRoot\bin\
    Copy-Item -Force -Recurse $projectRoot\vendor\cef\Release\*.bin               $packRoot\bin\
    Copy-Item -Force -Recurse $projectRoot\vendor\cef\Resources\icudtl.dat        $packRoot\bin\
    Copy-Item -Force -Recurse $projectRoot\vendor\cef\Resources\*.pak             $packRoot\bin\cef\
    Copy-Item -Force -Recurse $projectRoot\vendor\cef\Resources\locales\en-US.pak $packRoot\bin\cef\

    # remove CEF as redownloading is broken and this slows down gitlab ci cache
    Remove-Item -Recurse $projectRoot\vendor\cef\*

    if ($Context.IS_FIVEM) {
        Copy-Item -Force -Recurse $projectRoot\data\shared\*     $packRoot\
        Copy-Item -Force -Recurse $projectRoot\data\client\*     $packRoot\
        Copy-Item -Force -Recurse $projectRoot\data\redist\crt\* $packRoot\bin\
    }
    elseif ($Context.IS_REDM) {
        Copy-Item -Force -Recurse $projectRoot\data\shared\*                 $packRoot\
        Copy-Item -Force -Recurse $projectRoot\data\client\*.dll             $packRoot\
        Copy-Item -Force -Recurse $projectRoot\data\client\bin\*             $packRoot\bin\
        Copy-Item -Force -Recurse $projectRoot\data\redist\crt\*             $packRoot\bin\
        Copy-Item -Force -Recurse $projectRoot\data\client\citizen\clr2      $packRoot\citizen\
        Copy-Item -Force -Recurse $projectRoot\data\client\citizen\*.ttf     $packRoot\citizen\
        Copy-Item -Force -Recurse $projectRoot\data\client\citizen\ros       $packRoot\citizen\
        Copy-Item -Force -Recurse $projectRoot\data\client\citizen\resources $packRoot\citizen\
        Copy-Item -Force -Recurse $projectRoot\data\client_rdr\*             $packRoot\
    }

    # Adding build output
    Copy-Item -Force $binRoot\*.dll                      $packRoot\
    Copy-Item -Force $binRoot\*.com                      $packRoot\
    Copy-Item -Force $binRoot\CitizenFX_SubProcess_*.bin $packRoot\

    Copy-Item -Force -Recurse $binRoot\citizen\*         $packRoot\citizen\
	
    if ($Context.IS_FIVEM) {
        Copy-Item -Force $binRoot\VMP_Diag.exe $packRoot\
    }

    # Adding UI
    Copy-Item -Force $Context.getPathInProject("ext\ui-build\data.zip")     $packRoot\citizen\ui.zip
    Copy-Item -Force $Context.getPathInProject("ext\ui-build\data_big.zip") $packRoot\citizen\ui-big.zip
	
    # Writing versions
    $Versions.Game    | Out-File -Encoding ascii $packRoot\citizen\version.txt
    $Versions.BuildID | Out-File -Encoding ascii $packRoot\citizen\release.txt

    Write-ClientCacheVersions -CachesRoot $cachesRoot -CacheName $cacheName -Versions $Versions

    # GCI
    if ($Context.PrivateRoot) {
        # if (!$Tools.gci) {
        #     throw "No GCI tool"
        # }

        # $listFile = [IO.Path]::Combine($Context.PrivateRoot, "gci-list-{0}.txt" -f $Context.ProductName)

        # if (!(Test-Path $listFile)) {
        #     throw "No GCI list found"
        # }

        # & $Tools.gci make -r $packRoot -l $listFile
        # Test-LastExitCode "GCI tool failed"
    }

    # Bootstrapper
    $bootstrapperPath = [IO.Path]::Combine($binRoot, $Context.ProductExeName)
    $bootstrapperCompressedPath = "$cachesRoot\CitizenFX.exe.xz"

    Copy-Item -Force $bootstrapperPath $packRoot\CitizenFX.exe
    Copy-Item -Force $bootstrapperPath $cachesRoot\CitizenFX.exe

    Invoke-Something -Context $Context -PackRoot $packRoot

    # upload review jobs will use this compressed bootstrap file
    Remove-Item -Force -ErrorAction ignore $bootstrapperCompressedPath

    & $Tools.xz -9 $cachesRoot\CitizenFX.exe
    Test-LastExitCode "Failed to compress CitizenFX.exe with xz"

    $cachesVersion = @(
        $Versions.Launcher
        (Get-ItemProperty $bootstrapperCompressedPath).Length
    ) -join ' '
    $cachesVersion | Out-File -Encoding ascii $cachesRoot\version.txt
}
