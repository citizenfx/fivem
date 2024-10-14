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

    Remove-Item -Force -Recurse $symUploadDir
    New-Item -ItemType Directory -Force $symUploadDir

    Remove-Item -Force -Recurse $symPackDir
    New-Item -ItemType Directory -Force $symPackDir

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
    Push-Location $symUploadDir
        if ($Context.IsDryRun) {
            Write-Output "DRY RUN: Would upload symbols"
        } else {
            # remember current PATH content
            $oldEnvPath = $env:PATH
    
            # prepend PATH with MSYS2 bin dir so it can use it's own SSH
            $msys2UsrBin = [IO.Path]::Combine($Tools.getMSYS2Root(), "usr\bin");
            $env:PATH = "$msys2UsrBin;$env:PATH"
    
            & $rsync -r -a -v -e $env:RSH_SYMBOLS_COMMAND ./ $env:SSH_SYMBOLS_TARGET
            if ($LASTEXITCODE -ne 0) {
                $Context.addBuildWarning("Failed to upload symbols, rsync failed")
            }
    
            # restore PATH
            $env:PATH = $oldEnvPath
        }
    Pop-Location

    if ($Context.IsPublicBuild) {
        Invoke-SentryUploadDif -Context $Context -Tools $Tools -Path $symPackDir
    }
}
