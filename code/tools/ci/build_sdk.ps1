function Start-Section
{
    param([Parameter(Position=0)][string]$Id, [Parameter(Position=1)][string]$Name)
    
    $ts = (Get-Date -UFormat %s -Millisecond 0)
    $esc = $([char]27)
    Write-Host "section_start:$($ts):$Id`r$esc[0K$Name"
}

function End-Section
{
    param([Parameter(Position=0)][string]$Id)
    
    $ts = (Get-Date -UFormat %s -Millisecond 0)
    $esc = $([char]27)
    Write-Host "section_end:$($ts):$Id`r$esc[0K"
}

$ErrorActionPreference = "Stop"
$SaveDir = "C:\f\save"

$WorkDir = $env:CI_PROJECT_DIR -replace '/','\'
$WorkRootDir = "$WorkDir\code"

$FXCodeSubmodulePath = "$WorkDir\ext\sdk\resources\sdk-root\fxcode"
$LastFXCodeCommitPath = "$SaveDir\.fxcodecommit"
$LastFXCodeCommit = ""

if (Test-Path $LastFXCodeCommitPath) {
    $LastFXCodeCommit = Get-Content $LastFXCodeCommitPath
}

$ShouldBuildFXCode = $false

Push-Location $WorkDir
$SDKCommit = (git rev-list -1 HEAD ext/sdk-build/ ext/sdk/ code/tools/ci/build_sdk.ps1)
$SDKVersion = ((git rev-list $SDKCommit | measure-object).Count * 10) + 1100000
Pop-Location

Start-Section "update_submodules" "Updating submodules"
Push-Location $WorkDir

git submodule init
git submodule sync

Push-Location $WorkDir
$SubModules = git submodule | ForEach-Object { New-Object PSObject -Property @{ Hash = $_.Substring(1).Split(' ')[0]; Name = $_.Substring(1).Split(' ')[1] } }

foreach ($submodule in $SubModules) {
    $SubmodulePath = git config -f .gitmodules --get "submodule.$($submodule.Name).path"

    if ((Test-Path $SubmodulePath) -and (Get-ChildItem $SubmodulePath).Length -gt 0) {
        continue;
    }
    
    Start-Section "update_submodule_$($submodule.Name)" "Cloning $($submodule.Name)"
    $SubmoduleRemote = git config -f .gitmodules --get "submodule.$($submodule.Name).url"

    $Tag = (git ls-remote --tags $SubmoduleRemote | Select-String -Pattern $submodule.Hash | Select-Object -First 1) -replace '^.*tags/([^^]+).*$','$1'

    if (!$Tag) {
        git clone $SubmoduleRemote $SubmodulePath
    } else {
        git clone -b $Tag --depth 1 --single-branch $SubmoduleRemote $SubmodulePath
    }

    End-Section "update_submodule_$($submodule.Name)"
}
Pop-Location

Start-Section "update_submodule_git" "Updating all submodules"
git submodule update --jobs=8
End-Section "update_submodule_git"

Pop-Location

End-Section "update_submodules"

# Check if FXCode needs building
Push-Location $FXCodeSubmodulePath
$FXCodeSubmoduleCommit = git rev-parse HEAD

if ($FXCodeSubmoduleCommit -ne $LastFXCodeCommit) {
    $LastFXCodeCommit = $FXCodeSubmoduleCommit
    $ShouldBuildFXCode = $true
    $LastFXCodeCommit | Out-File -Encoding ascii -NoNewline $LastFXCodeCommitPath
}
Pop-Location

# start building SDK
# copied from run_postbuild.ps1
Push-Location $WorkDir\ext\sdk-build
if (!(Test-Path sdk-root\.commit) -or $SDKCommit -ne (Get-Content sdk-root\.commit)) {
    .\build.cmd --build-fxcode=$ShouldBuildFXCode --fxcode-commit=$LastFXCodeCommit
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
# xcopy /y /e $WorkDir\ext\sdk\resources\sdk-game\*.* $CacheDir\fxdk-five\citizen\sdk\sdk-game\
xcopy /y /e $WorkDir\ext\sdk\resources\sdk-game\fxmanifest.lua $CacheDir\fxdk-five\citizen\sdk\sdk-game\
xcopy /y /e $WorkDir\ext\sdk\resources\sdk-game\sdk-client.js $CacheDir\fxdk-five\citizen\sdk\sdk-game\
xcopy /y /e $WorkDir\ext\sdk\resources\sdk-game\sdk-server.js $CacheDir\fxdk-five\citizen\sdk\sdk-game\

"<Caches>
<Cache ID=`"fxdk-five`" Version=`"$SDKVersion`" />
</Caches>" | Out-File -Encoding ascii $CacheDir\caches.xml

Copy-Item -Force "$WorkRootDir\tools\ci\xz.exe" xz.exe

Move-Item -Force $CacheDir\caches.xml $CacheDir\caches_sdk.xml

if (Test-Path $WorkDir\caches) {
    Remove-Item -Recurse -Force $WorkDir\caches
}

Copy-Item -Recurse -Force $CacheDir $WorkDir\caches
