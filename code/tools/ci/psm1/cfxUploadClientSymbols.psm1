using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1

function Invoke-UploadClientSymbols {
    param(
        [CfxBuildContext] $Context,
        [CfxBuildTools] $Tools
    )

    $rsync = $Tools.getRsync()
    $symstore = $Tools.getSymstore()
    $dump_syms = $Tools.getDumpSyms()

    $symTmpDir = $Context.getPathInBuildCache("symbols\sym-tmp")
    $symUploadDir = $Context.getPathInBuildCache("symbols\sym-upload")
    $lastSymDateFilePath = $Context.getPathInBuildCache("symbols\last-upload-datetime.txt")

    Remove-Item -Force -Recurse $symUploadDir
    New-Item -ItemType Directory -Force $symUploadDir

    Remove-Item -Force -Recurse $symTmpDir
    New-Item -ItemType Directory -Force $symTmpDir

    # setting a really old date by default so if no symbols have been dumped before we'd dump them now
    $lastSymDate = Get-Date -Year 1

    if (Test-Path $lastSymDateFilePath) {
        try {
            $lastSymDate = [DateTime](Get-Content $lastSymDateFilePath)
        }
        catch {
            # noop
        }
    }

    foreach ($file in (Get-ChildItem -Recurse $Context.MSBuildOutput)) {
        # only need files with these extensions
        if ($file.Extension -notin @(".dll", ".pdb", ".exe", ".bin")) {
            continue
        }

        # ignore v8 snapshot_blob.bin file
        if ($file.Name -eq "snapshot_blob.bin") {
            continue
        }

        # don't process symbols that has been already processed while running this last time
        if ($file.LastWriteTime -le $lastSymDate) {
            continue
        }

        Push-Location $Context.MSBuildOutput
            $relPath = (Resolve-Path -Relative $file.FullName) -replace "dbg\\", ""
        Pop-Location

        $relDir = [IO.Path]::GetDirectoryName($relPath)

        $fileDir = [IO.Path]::Combine($symTmpDir, $relDir)
        $filePath = [IO.Path]::Combine($symTmpDir, $relPath)

        New-Item -ItemType Directory -Force -Path $fileDir
        Copy-Item -Force -Path $file.FullName -Destination $filePath
    }

    & $symstore add /o /f $symTmpDir /s $symUploadDir /t "Cfx" /r
    Test-LastExitCode "Failed to upload symbols, symstore failed"
    
    $pdbs  = @(Get-ChildItem -Recurse -Filter "*.dll" -File $symUploadDir)
    $pdbs += @(Get-ChildItem -Recurse -Filter "*.exe" -File $symUploadDir)
    $pdbs += @(Get-ChildItem -Recurse -Filter "*.bin" -File $symUploadDir)

    $pdbs = $pdbs.Where{ $_.BaseName -notin @("botan", "citizen-scripting-lua", "citizen-scripting-lua54") }

    foreach ($pdb in $pdbs) {
        $outname = [io.path]::ChangeExtension($pdb.FullName, "sym")

        Start-Process $dump_syms -ArgumentList ($pdb.FullName) -RedirectStandardOutput $outname -Wait -WindowStyle Hidden
    }

    foreach ($pdb in $pdbs) {
        $basename = $pdb.BaseName
        $outname = [IO.Path]::ChangeExtension($pdb.FullName, "sym")
    
        $success = $false
    
        while (!$success) {
            try {
                try {
                    $reader = [System.IO.File]::OpenText($outname)
                    $line = $reader.ReadLine() -split " "
                } finally {
                    $reader.Close()
                }                    

                if ($line[0] -eq "MODULE") {
                    $rightname = "$($symUploadDir)\$($line[4])\$($line[3])\$basename.sym"

                    if (Test-Path ([IO.Path]::GetDirectoryName($rightname))) {
                        Move-Item $outname $rightname
                    }
                }

                $success = $true
            } catch {
                Start-Sleep -Milliseconds 500
            }
        }
    }

    # chdir to the directory to avoid converting path to what rsync would expect
    Push-Location $symUploadDir
        if ($Context.IsDryRun) {
            Write-Output "DRY RUN: Would upload symbols"

            (Get-Date).ToString() | Out-File -Encoding ascii $lastSymDateFilePath
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
            else {
                (Get-Date).ToString() | Out-File -Encoding ascii $lastSymDateFilePath
            }

            # restore PATH
            $env:PATH = $oldEnvPath
        }
    Pop-Location

    if (!$Context.IsDryRun) {
        # Cleanup
        Remove-Item -Force -Recurse $symUploadDir
        Remove-Item -Force -Recurse $symTmpDir
    }
}
