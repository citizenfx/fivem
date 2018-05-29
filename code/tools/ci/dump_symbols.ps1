
param (
    [string]
    $BinRoot
)

remove-item -recurse -force R:\sym-upload
remove-item -recurse -force R:\sym-pack
mkdir R:\sym-upload
mkdir R:\sym-pack
C:\h\debuggers\symstore.exe add /o /f $BinRoot\five\release\ /s R:\sym-upload /t "Cfx" /r

$pdbs = Get-ChildItem -Recurse -Filter "*.pdb" -File R:\sym-upload\

workflow dump_pdb {
    param ([Object[]]$files)

    foreach -parallel ($pdb in $files) {
        sequence {
            $basename = $pdb.BaseName
            $outname = [io.path]::ChangeExtension($pdb.FullName, "sym")

            if (!($basename -eq "botan")) {
                Start-Process C:\f\dump_syms.exe -ArgumentList ($pdb.FullName) -RedirectStandardOutput $outname -Wait -WindowStyle Hidden
            }
        }
    }
}

dump_pdb -files $pdbs

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
