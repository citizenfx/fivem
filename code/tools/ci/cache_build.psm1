$BuildCacheMeta = "https://github.com/citizenfx/buildcachemeta-go/releases/download/v0.0.2/buildcachemeta-go_0.0.2_win32_amd64.tar.gz"

function Invoke-CacheGen {
	param(
		[string] $Source,
		[string] $CacheName,
		[string] $BranchName,
		[int] $BranchVersion,
		[string] $BootstrapName,
		[int] $BootstrapVersion
	)

	curl.exe -Lo $env:TEMP\bcm.tar.gz $BuildCacheMeta
	& "$env:WINDIR\system32\tar.exe" -C "$env:TEMP" -xvf "$env:TEMP\bcm.tar.gz"
	& "$env:TEMP\buildcachemeta-go" -s3-endpoint $env:CACHE_S3_ENDPOINT -s3-key-id $env:CACHE_S3_KEY_ID -s3-key $env:CACHE_S3_KEY -s3-name $env:CACHE_S3_BUCKET `
		-source "$Source" -branch-name "$BranchName" -branch-version "$BranchVersion" -bootstrap-executable "$BootstrapName" -bootstrap-version "$BootstrapVersion"

	if (!$?) {
		throw "Failed to upload!"
	}
}
