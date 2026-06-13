using module .\cfxBuildContext.psm1
using module .\cfxBuildTools.psm1
using module .\cfxSentry.psm1

function Rename-SymstoreEntry {
    param(
        [string] $SymUploadDir,
        [string] $OldName,
        [string] $NewName
    )

    $sourceDir = [IO.Path]::Combine($SymUploadDir, $OldName)
    if (!(Test-Path $sourceDir)) {
        return
    }

    foreach ($hashDir in (Get-ChildItem -Directory -Path $sourceDir)) {
        $sourceFile = [IO.Path]::Combine($hashDir.FullName, $OldName)
        if (Test-Path $sourceFile) {
            Rename-Item -Path $sourceFile -NewName $NewName
        }
    }

    Rename-Item -Path $sourceDir -NewName $NewName
}

function Copy-SymstoreEntry {
    param(
        [string] $SymUploadDir,
        [string] $OldName,
        [string] $NewName
    )

    $sourceDir = [IO.Path]::Combine($SymUploadDir, $OldName)
    if (!(Test-Path $sourceDir)) {
        return
    }

    $destDir = [IO.Path]::Combine($SymUploadDir, $NewName)
    Copy-Item -Recurse -Force -Path $sourceDir -Destination $destDir

    foreach ($hashDir in (Get-ChildItem -Directory -Path $destDir)) {
        $sourceFile = [IO.Path]::Combine($hashDir.FullName, $OldName)
        if (Test-Path $sourceFile) {
            Rename-Item -Path $sourceFile -NewName $NewName
        }
    }
}

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

    # Rename .bin to .exe so the symbol store module names match the client
    foreach ($binFile in (Get-ChildItem -Recurse -Filter "*.bin" -File $symTmpDir)) {
        $exePath = [IO.Path]::ChangeExtension($binFile.FullName, ".exe")
        Move-Item -Force -Path $binFile.FullName -Destination $exePath
    }

    & $symstore add /o /f $symTmpDir /s $symUploadDir /t "Cfx" /r
    Test-LastExitCode "Failed to upload symbols, symstore failed"
    
    $pdbs  = @(Get-ChildItem -Recurse -Filter "*.dll" -File $symUploadDir)
    $pdbs += @(Get-ChildItem -Recurse -Filter "*.exe" -File $symUploadDir)

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

    # Rename symbol server entries to match runtime names
    $aliasPrefix = if ($Context.IS_FIVEM) { "FiveM" } elseif ($Context.IS_REDM) { "RedM" } else { $null }

    if ($aliasPrefix) {
        # Game executables
        foreach ($sourceDir in (Get-ChildItem -Directory -Path $symUploadDir -Filter "CitizenFX_SubProcess_game_*_aslr.exe")) {
            if ($sourceDir.Name -match 'CitizenFX_SubProcess_game_(\d+)_aslr\.exe') {
                $buildNum = $Matches[1]
                Rename-SymstoreEntry $symUploadDir $sourceDir.Name "${aliasPrefix}_b${buildNum}_GTAProcess.exe"
            }
        }

        # CitizenFX_SubProcess_chrome -> ChromeBrowser
        Rename-SymstoreEntry $symUploadDir "CitizenFX_SubProcess_chrome.exe" "${aliasPrefix}_ChromeBrowser"

        # CitizenFX_SubProcess_game_mtl -> ROSLauncher + ROSService
        Copy-SymstoreEntry $symUploadDir "CitizenFX_SubProcess_game_mtl.exe" "${aliasPrefix}_ROSService"
        Rename-SymstoreEntry $symUploadDir "CitizenFX_SubProcess_game_mtl.exe" "${aliasPrefix}_ROSLauncher"

        # Main exe -> DumpServer (copy)
        Copy-SymstoreEntry $symUploadDir "${aliasPrefix}.exe" "${aliasPrefix}_DumpServer"
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

    if ($Context.IsPublicBuild) {
        Invoke-SentryUploadDebugFiles -Context $Context -Tools $Tools -Path $symUploadDir
    }
    
    if (!$Context.IsDryRun) {
        # Cleanup
        Remove-Item -Force -Recurse $symUploadDir
        Remove-Item -Force -Recurse $symTmpDir
    }
}
