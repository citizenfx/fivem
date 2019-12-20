
param (
    [string]
    $BinRoot,

    [string]
    $GameName
)

$lastSymDate = (Get-Item R:\sym-pack.zip).LastWriteTime

remove-item -recurse -force R:\sym-upload
remove-item -recurse -force R:\sym-pack
remove-item -recurse -force R:\sym-tmp
mkdir R:\sym-upload
mkdir R:\sym-pack
mkdir R:\sym-tmp

foreach ($file in (Get-ChildItem -Recurse $BinRoot\$GameName\release)) {
    if ($file.LastWriteTime -ge $lastSymDate) {
        if ($file.Extension -in @(".dll", ".pdb", ".exe")) {
            Push-Location $BinRoot\$GameName\release
            $relPath = (Resolve-Path -Relative $file.FullName)
            Pop-Location

            $relDir = [IO.Path]::GetDirectoryName($relPath)

            New-Item -ItemType Directory -Force "R:\sym-tmp\$($relDir)"
            Copy-Item -Force $file.FullName "R:\sym-tmp\$($relPath)"
        }
    }
}

C:\h\debuggers\symstore.exe add /o /f R:\sym-tmp /s R:\sym-upload /t "Cfx" /r

$pdbs  = @(Get-ChildItem -Recurse -Filter "*.dll" -File R:\sym-upload\)
$pdbs += @(Get-ChildItem -Recurse -Filter "*.exe" -File R:\sym-upload\)

workflow dump_pdb {
    param ([Object[]]$files)

    foreach -parallel -throttlelimit 10 ($pdb in $files) {
        $basename = $pdb.BaseName
        $outname = [io.path]::ChangeExtension($pdb.FullName, "sym")

        if (!($basename -eq "botan") -and !($basename -eq "citizen-scripting-lua")) {
            Start-Process C:\f\dump_syms.exe -ArgumentList ($pdb.FullName) -RedirectStandardOutput $outname -Wait -WindowStyle Hidden
        }
    }
}

dump_pdb -files $pdbs

foreach ($pdb in $pdbs) {
    $basename = $pdb.BaseName
    $outname = [io.path]::ChangeExtension($pdb.FullName, "sym")

    if (!($basename -eq "botan") -and !($basename -eq "citizen-scripting-lua")) {
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
                    $rightname = "R:\sym-upload\$($line[4])\$($line[3])\$basename.sym"
                    Move-Item $outname $rightname
                }

                $success = $true
            } catch {
                Start-Sleep -Milliseconds 500
            }
        }
    }
}

$syms = Get-ChildItem -Recurse -Filter "*.sym" -File R:\sym-upload\

foreach ($sym in $syms) {
    $baseDir = $sym.FullName -replace "sym-upload","sym-pack"
    mkdir ([io.path]::GetDirectoryName($baseDir))
    copy-item $sym.FullName $baseDir
}

$env:Path = "C:\msys64\usr\bin;$env:Path"

remove-item R:\sym-pack.zip
C:\f\temp\7z.exe a R:\sym-pack.zip R:\sym-pack\*
C:\f\temp\curl.exe -F symbols=@R:\sym-pack.zip -F version=1.0 -F platform=windows https://crashes.fivem.net/symbols/
rsync -r -a -v -e $env:RSH_COMMAND /r/sym-upload/ $env:SSH_SYMBOLS_TARGET
