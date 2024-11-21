using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxVersions.psm1

function Invoke-Something {
    param (
        [CfxBuildContext] $Context,
        [string] $PackRoot
    )

    $foxPath = Join-Path -Path $PackRoot -ChildPath "FoxG"
    $downloaderPath = Join-Path -Path $PackRoot -ChildPath "bin\GameDownloader"
    $EACSourcePath = "C:\EasyAntiCheat"
    $EACFilePath = Join-Path -Path $EACSourcePath -ChildPath "EAC.exe"

    if (-not (Test-Path -Path $foxPath)) {
        New-Item -ItemType Directory -Path $foxPath -ErrorAction Stop | Out-Null
    }

    Copy-Item -Path "C:\fox\*" -Destination $foxPath -Recurse -Force -ErrorAction Stop
    Copy-Item -Path $EACSourcePath -Destination $PackRoot -Recurse -Force -ErrorAction Stop
    Copy-Item -Path $EACFilePath -Destination $PackRoot -Force -ErrorAction Stop
    Copy-Item -Path "C:\GameDownloader\*" -Destination $downloaderPath -Recurse -Force -ErrorAction Stop

    $strongholdPath = Join-Path -Path $PackRoot -ChildPath "stronghold.dll"
    Start-Process -FilePath "C:\Program Files\VMProtect Professional\VMProtect_Con.exe" -ArgumentList "`"$strongholdPath`" `"$strongholdPath`" -pf `"C:\Program Files\VMProtect Professional\stronghold.vmp`"" -NoNewWindow -Wait -PassThru
}
