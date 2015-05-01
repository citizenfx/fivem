Invoke-WebRequest "http://heanet.dl.sourceforge.net/project/premake/Premake/nightlies/premake-stable-windows.zip" -OutFile "build\premake.zip"

try {
    [System.Reflection.Assembly]::LoadWithPartialName("System.IO.Compression.FileSystem") | Out-Null
} catch {
    Write-Error -Message "Could not load System.IO.Compression.FileSystem -- is .NET 4.5 installed?"
    return
}

if (Test-Path "build\premake4.exe") {
    Remove-Item "build\premake4.exe"
}

$zipfile = [System.IO.Compression.ZipFile]::OpenRead("build\premake.zip")
$file = $zipfile.GetEntry("bin/release/premake4.exe")
[System.IO.Compression.ZipFileExtensions]::ExtractToFile($file, "build\premake4.exe")