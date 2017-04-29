Invoke-WebRequest "https://github.com/premake/premake-core/releases/download/v5.0.0-alpha6/premake-5.0.0-alpha6-windows.zip" -OutFile "build\premake.zip"

try {
    [System.Reflection.Assembly]::LoadWithPartialName("System.IO.Compression.FileSystem") | Out-Null
} catch {
    Write-Error -Message "Could not load System.IO.Compression.FileSystem -- is .NET 4.5 installed?"
    return
}

if (Test-Path "build\premake5.exe") {
    Remove-Item "build\premake5.exe"
}

$zipfile = [System.IO.Compression.ZipFile]::OpenRead("build\premake.zip")
$file = $zipfile.GetEntry("premake5.exe")
[System.IO.Compression.ZipFileExtensions]::ExtractToFile($file, "build\premake5.exe")
