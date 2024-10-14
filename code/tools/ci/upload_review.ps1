$ErrorActionPreference = 'Stop'

$WorkDir = $env:CI_PROJECT_DIR -replace '/','\'
$WorkRootDir = "$WorkDir\code"
$CacheDir = "$WorkDir\caches"

$PackFromRoot = "$env:TEMP\FxUpload-${env:CI_PIPELINE_ID}-${env:CI_JOB_ID}"

$PackToName = "FxUpload-${env:CI_PIPELINE_ID}-${env:CI_JOB_NAME_SLUG}.zip"
$PackToPath = "$env:TEMP\$PackToName"

Remove-Item -Force -Recurse -ErrorAction ignore $PackFromRoot
Remove-Item -Force -ErrorAction ignore $PackToPath

New-Item -Force -ItemType Directory $PackFromRoot
& "$WorkRootDir\tools\ci\7z.exe" x -y "-o$PackFromRoot" "$CacheDir\CitizenFX.exe.xz"

$items = (Get-ChildItem -Recurse -Path $CacheDir)

foreach ($item in $items) {
	if (Test-Path $Item.FullName -NewerThan 1/1/2020) {
		if ($item.Name.EndsWith(".dll.xz") -or $item.Name.EndsWith(".exe.xz") -or $item.Name.EndsWith(".bin.xz")) {
			& $WorkRootDir\tools\ci\7z.exe x -y "-o$PackFromRoot" $item.FullName
		} elseif ($item.Name.EndsWith(".dll") -or $item.Name.EndsWith(".exe") -or $item.Name.EndsWith(".bin")) {
			Copy-Item -Force -Path $item.FullName -Destination $PackFromRoot
		}
	}
}

& "$WorkRootDir\tools\ci\7z.exe" a -mx=5 $PackToPath $PackFromRoot

if ($env:CFX_DRY_RUN -eq "true") {
	Write-Output "DRY RUN: Would upload review"
} else {
	& "curl.exe" --fail-with-body -T $PackToPath "${env:REVIEW_UPLOAD_URL}/$PackToName"
	if ($LASTEXITCODE -ne 0) {
		Write-Host "Failed to upload review"
		Exit 1
	}
}

Remove-Item -Force -Recurse $PackFromRoot
Remove-Item -Force $PackToPath
