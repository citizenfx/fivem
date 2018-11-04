
param (
    [string]
    $BinRoot
)

mkdir C:\f\work

remove-item -recurse -force c:\f\work\sym-upload
remove-item -recurse -force c:\f\work\sym-pack
mkdir C:\f\work\sym-upload
mkdir C:\f\work\sym-pack
C:\h\debuggers\x64\symstore.exe add /o /f $BinRoot\server\windows\release\ /s C:\f\work\sym-upload /t "Cfx" /r

$dlls = Get-ChildItem -Recurse -Filter "*.dll" -File $BinRoot\server\windows\release\
$exes = Get-ChildItem -Recurse -Filter "*.exe" -File $BinRoot\server\windows\release\

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

dump_pdb -files $dlls
dump_pdb -files $exes

$syms = Get-ChildItem -Recurse -Filter "*.sym" -File $BinRoot\server\windows\release\

foreach ($sym in $syms) {
    if ($sym.Length -gt 0) {
        copy-item $sym.FullName C:\f\work\sym-pack\
    }
}

$env:Path = "C:\msys64\usr\bin;$env:Path"

sentry-cli upload-dif --org citizenfx --project fxserver C:\f\work\sym-pack\
rsync -r -a -v -e $env:RSH_COMMAND /c/f/work/sym-upload/ $env:SSH_SYMBOLS_TARGET
