$ErrorActionPreference = 'Stop'

$WorkDir = $env:CI_PROJECT_DIR -replace '/','\'
$WorkRootDir = "$WorkDir\code"
$CacheDir = "$WorkDir\caches"

$TempDir = "$env:TEMP\FxUpload"

$OutName = "FxUpload-${env:CI_PIPELINE_ID}-${env:CI_JOB_NAME}.zip"
$OutZip = "$TempDir\..\$OutName"

if (Test-Path $TempDir) {
	Remove-Item -Force -Recurse $TempDir
}

if (Test-Path $OutZip) {
	Remove-Item -Force $OutZip
}

New-Item -Force -ItemType Directory $TempDir
& "$WorkRootDir\tools\ci\7z.exe" x -y "-o$TempDir" "$CacheDir\CitizenFX.exe.xz"

$items = (Get-ChildItem -Recurse -Path $CacheDir)

foreach ($item in $items) {
	if (Test-Path $Item.FullName -NewerThan 1/1/2020) {
		if ($item.Name.EndsWith(".dll.xz") -or $item.Name.EndsWith(".exe.xz") -or $item.Name.EndsWith(".bin.xz")) {
			& $WorkRootDir\tools\ci\7z.exe x -y "-o$TempDir" $item.FullName
		} elseif ($item.Name.EndsWith(".dll") -or $item.Name.EndsWith(".exe") -or $item.Name.EndsWith(".bin")) {
			Copy-Item -Force -Path $item.FullName -Destination $TempDir
		}
	}
}

& "$WorkRootDir\tools\ci\7z.exe" a -mx=5 $OutZip $TempDir
& "curl.exe" -T $OutZip "${env:REVIEW_UPLOAD_URL}/$OutName"

Remove-Item -Force -Recurse $TempDir
Remove-Item -Force $OutZip
