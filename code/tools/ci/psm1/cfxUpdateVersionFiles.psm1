using module .\cfxBuildContext.psm1
using module .\cfxVersions.psm1

function Invoke-UpdateVersionFiles {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions
    )

    $CfxVersionFile = $Context.getPathInProject("code\shared\cfx_version.h")
    $CitVersionFile = $Context.getPathInProject("code\shared\citversion.h")
    $LauncherVersionFile = $Context.getPathInProject("code\shared\launcher_version.h")

    $expectedCitVersionFileContent = @(
        "#pragma once"
        "#define BASE_EXE_VERSION {0}" -f $Versions.Launcher
    ) -join "`n"

    $actualCitVersionFileContent = Get-Content -Raw -ErrorAction ignore $CitVersionFile

    $citVersionFileContentDifference = Compare-Object ($expectedCitVersionFileContent -replace '\s','') ($actualCitVersionFileContent -replace '\s','')

    if ($null -ne $citVersionFileContentDifference) {
        $expectedCitVersionFileContent | Out-File -Force -Encoding ascii $CitVersionFile

        if ($Versions.BuildID) {
            @(
                "#pragma once"
                "#define EXE_VERSION {0}" -f $Versions.BuildID
            ) -join "`n" | Out-File -Force -Encoding ascii $LauncherVersionFile
        }
    }

    $gitDescription = @(
        $Context.GitBranchName
    )

    if (!$Context.IS_FIVEM) {
        $gitDescription += $Context.PremakeGameName.ToUpper()
    }

    if ($Context.GitTag) {
        $gitDescription += $Context.GitTag
    } elseif ($Context.GitBranchName.StartsWith("feature/")) {
        $dateVersion = (Get-Date).ToString("yyyyMMdd");
        $gitDescription += "v1.0.0.$dateVersion";
    }

    @(
        "#pragma once"
        '#define GIT_DESCRIPTION "{0} win32"' -f ($gitDescription -join " ")
        '#define GIT_TAG "{0}"' -f $Context.GitTag
    ) -join "`n" | Out-File -Force -Encoding ascii $CfxVersionFile
}