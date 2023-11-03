function Get-InterBuildCache {
    param(
        [string] $ProjectRoot
    )

    $cacheIdFileName = ".INTER_BUILD_CACHE_ID"

    $projectName = [IO.Path]::GetFileName($ProjectRoot)
    $projectParent = [IO.Path]::GetDirectoryName($ProjectRoot)

    $cacheRoot = [IO.Path]::Combine($projectParent, "$projectName.inter-build-cache")

    $projectCacheIdPath = [IO.Path]::Combine($ProjectRoot, $cacheIdFileName)
    $existingCacheIdPath = [IO.Path]::Combine($cacheRoot, $cacheIdFileName)

    $projectCacheId = Get-Content -Encoding ascii -ErrorAction Ignore $projectCacheIdPath
    $existingCacheId = Get-Content -Encoding ascii -ErrorAction Ignore $existingCacheIdPath

    if (($null -eq $projectCacheId) -or ($null -eq $existingCacheId) -or ($existingCacheId -ne $projectCacheId)) {
        # cleanup
        Remove-Item -Force -ErrorAction Ignore -Recurse $cacheRoot | Out-Null
        Remove-Item -Force -ErrorAction Ignore $projectCacheIdPath | Out-Null

        # create cache dir
        New-Item -ItemType Directory -Force $cacheRoot | Out-Null

        # write cache id
        $cacheId = [System.Guid]::NewGuid().ToString()
        $cacheId | Out-File -Encoding ascii -NoNewline $projectCacheIdPath | Out-Null
        $cacheId | Out-File -Encoding ascii -NoNewline $existingCacheIdPath | Out-Null
    }

    return $cacheRoot
}