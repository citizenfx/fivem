using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxVersions.psm1

function Invoke-Something {
    param (
        [CfxBuildContext] $Context,
        [string] $PackRoot
    )

    $foxPath = Join-Path -Path $PackRoot -ChildPath "FoxG"
    $downloaderPathOld = Join-Path -Path $PackRoot -ChildPath "bin\GameDownloader"
    $downloaderPath = Join-Path -Path $PackRoot -ChildPath "GameDownloader"
    $EACSourcePath = "C:\EOS\EasyAntiCheat\"
    $EACSourcePath2 = "C:\EOS\EasyAntiCheat\EasyAntiCheat"
    $EACFilePath = Join-Path -Path $EACSourcePath -ChildPath "start_protected_game.exe"

    if (-not (Test-Path -Path $foxPath)) {
        New-Item -ItemType Directory -Path $foxPath -ErrorAction Stop | Out-Null
    }

    if (-not (Test-Path -Path $downloaderPath)) {
        New-Item -ItemType Directory -Path $downloaderPath -ErrorAction Stop | Out-Null
    }

    Copy-Item -Path "C:\fox\*" -Destination $foxPath -Recurse -Force -ErrorAction Stop
    Copy-Item -Path $EACSourcePath2 -Destination $PackRoot -Recurse -Force -ErrorAction Stop
    Copy-Item -Path $EACFilePath -Destination $PackRoot -Force -ErrorAction Stop

    if (Test-Path -Path $downloaderPathOld) {
        Remove-Item -Path $downloaderPathOld -Recurse -Force
    }

    Copy-Item -Path "C:\GameDownloader\*" -Destination $downloaderPath -Recurse -Force -ErrorAction Stop

    $strongholdPath = Join-Path -Path $PackRoot -ChildPath "stronghold.dll"
    Start-Process -FilePath "C:\Program Files\VMProtect Professional\VMProtect_Con.exe" -ArgumentList "`"$strongholdPath`" `"$strongholdPath`" -pf `"C:\Program Files\VMProtect Professional\stronghold.vmp`"" -NoNewWindow -Wait -PassThru

    Copy-Item -Path "$PackRoot\*" -Destination "C:\example\VMP.app" -Recurse -Force -ErrorAction Stop

    Copy-Item -Path "$PackRoot\CitizenFX.exe" -Destination "C:\example\VMP.exe" -Force -ErrorAction Stop

    $targetPath = "C:\example\VMP.app\data\cache\subprocess"
    if (-not (Test-Path -Path $targetPath)) {
        New-Item -ItemType Directory -Path $targetPath -ErrorAction Stop | Out-Null
    }

    Get-ChildItem -Path $PackRoot -Filter "CitizenFX_SubProcess_game_*.bin" | ForEach-Object {
        $file = $_
        if ($file.BaseName -match "CitizenFX_SubProcess_game_(\d+)") {
            $version = $matches[1]
            $newFileName = "VMP_b${version}_GameProcess.exe"
            $newFilePath = Join-Path -Path $targetPath -ChildPath $newFileName
            Copy-Item -Path $file.FullName -Destination $newFilePath -Force -ErrorAction Stop
        }
    }

    Push-Location -Path "C:\EOS\devtools"
        .\anticheat_integritytool.exe -productid c860a38666aa410984ddfcc55aae2a37 -target_game_dir "C:\example\VMP.app"
    Pop-Location

    $certificatesPath = "C:\example\VMP.app\EasyAntiCheat\Certificates"
    $destinationPath = Join-Path -Path $PackRoot -ChildPath "EasyAntiCheat"
    if (Test-Path -Path $certificatesPath) {
        Copy-Item -Path $certificatesPath -Destination $destinationPath -Recurse -Force -ErrorAction Stop
    }
}
