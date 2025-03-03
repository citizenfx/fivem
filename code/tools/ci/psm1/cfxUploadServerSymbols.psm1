using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxSentry.psm1

function Invoke-UploadServerSymbols {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools
    )

    $rsync = $Tools.getRsync()
    $symstore = $Tools.getSymstore()
    $dump_syms = $Tools.getDumpSyms()

    $symPackDir = $Context.getPathInBuildCache("symbols\sym-pack")
    $symUploadDir = $Context.getPathInBuildCache("symbols\sym-upload")
    $symUploadDirLower = $Context.getPathInBuildCache("symbols\sym-upload2")

    Remove-Item -Force -Recurse $symUploadDir
    New-Item -ItemType Directory -Force $symUploadDir

    Remove-Item -Force -Recurse $symPackDir
    New-Item -ItemType Directory -Force $symPackDir

    Remove-Item -Force -Recurse $symUploadDirLower
    New-Item -ItemType Directory -Force $symUploadDirLower


    & $symstore add /o /f ($Context.MSBuildOutput) /s $symUploadDir /t "Cfx" /r
    Test-LastExitCode "Failed to upload symbols, symstore failed"
    
    $pdbs  = @(Get-ChildItem -Recurse -Filter "*.dll" -File ($Context.MSBuildOutput))
    $pdbs += @(Get-ChildItem -Recurse -Filter "*.exe" -File ($Context.MSBuildOutput))

    $pdbs = $pdbs.Where{ $_.BaseName -notin @("botan") }

    foreach ($pdb in $pdbs) {
        $outname = [io.path]::ChangeExtension($pdb.FullName, "sym")

        Start-Process $dump_syms -ArgumentList ($pdb.FullName) -RedirectStandardOutput $outname -Wait -WindowStyle Hidden
    }

    $syms = Get-ChildItem -Recurse -Filter "*.sym" -File ($Context.MSBuildOutput)

    foreach ($sym in $syms) {
        if ($sym.Length -gt 0) {
            Copy-Item $sym.FullName $symPackDir\
        }
    }

    # chdir to the directory to avoid converting path to what rsync would expect
    $items = Get-ChildItem -Path $symUploadDir -Recurse

    foreach ($item in $items) {
        $newName = $item.FullName.ToLower()

        $destination = $newName -replace [regex]::Escape($symUploadDir), $symUploadDirLower

        if ($item.PSIsContainer) {
            New-Item -ItemType Directory -Path $destination -Force
        }
        else {
            Copy-Item -Path $item.FullName -Destination $destination -Force
        }
    }

    Push-Location $symUploadDirLower
        & $env:MC_PATH mirror --overwrite ./ $env:SYMBOL_PATH
        if ($LASTEXITCODE -ne 0) {
            $Context.addBuildWarning("Failed to upload symbols, mc mirror failed")
        }
    Pop-Location


    # if ($Context.IsPublicBuild) {
    #     Invoke-SentryUploadDif -Context $Context -Tools $Tools -Path $symPackDir
    # }
}
