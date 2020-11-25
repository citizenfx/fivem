$ErrorActionPreference = "Stop"
$SaveDir = "C:\f\save"

$WorkDir = $env:CI_PROJECT_DIR -replace '/','\'
$WorkRootDir = "$WorkDir\code"

Push-Location $WorkDir
$SDKCommit = (git rev-list -1 HEAD ext/sdk-build/ ext/sdk/ code/tools/ci/build_sdk.ps1)
$SDKVersion = ((git rev-list $SDKCommit | measure-object).Count * 10) + 1100000
Pop-Location

# start building SDK
# copied from run_postbuild.ps1
Push-Location $WorkDir\ext\sdk-build
if (!(Test-Path sdk-root\.commit) -or $SDKCommit -ne (Get-Content sdk-root\.commit)) {
    .\build.cmd

    if (!$?) {
        throw "Build failed!";
    }
    
    $SDKCommit | Out-File -Encoding ascii -NoNewline sdk-root\.commit
}

Pop-Location

# create save directory
New-Item -ItemType Directory -Force $SaveDir | Out-Null
$CacheDir = "$SaveDir\caches"

# prepare caches
New-Item -ItemType Directory -Force $CacheDir | Out-Null
New-Item -ItemType Directory -Force $CacheDir\fxdk-five | Out-Null
New-Item -ItemType Directory -Force $CacheDir\fxdk-five\citizen | Out-Null
New-Item -ItemType Directory -Force $CacheDir\fxdk-five\citizen\sdk | Out-Null
Set-Location $CacheDir

robocopy $WorkDir\ext\sdk-build\sdk-root\resource\ $CacheDir\fxdk-five\citizen\sdk\sdk-root\ /mir /xo /fft /ndl /njh /njs /nc /ns /np
xcopy /y /e $WorkDir\ext\sdk\resources\sdk-game\*.* $CacheDir\fxdk-five\citizen\sdk\sdk-game\

"<Caches>
<Cache ID=`"fxdk-five`" Version=`"$SDKVersion`" />
</Caches>" | Out-File -Encoding ascii $CacheDir\caches.xml

Copy-Item -Force "$WorkRootDir\tools\ci\xz.exe" xz.exe

Invoke-Expression "& $WorkRootDir\tools\ci\BuildCacheMeta.exe"

Move-Item -Force $CacheDir\caches.xml $CacheDir\caches_sdk.xml

if (Test-Path $WorkDir\caches) {
    Remove-Item -Recurse -Force $WorkDir\caches
}

Copy-Item -Recurse -Force $CacheDir $WorkDir\caches