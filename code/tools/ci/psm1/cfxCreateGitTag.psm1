using module .\cfxBuildContext.psm1
using module .\cfxVersions.psm1

function Invoke-CreateGitTag {
    param(
        [CfxBuildContext] $Context,
        [CfxVersions] $Versions
    )
    
    $tag = "v1.0.0.{0}" -f $Versions.BuildID
    $Context.GitTag = $tag

    if (!$env:GITHUB_CRED) {
        $Context.addBuildWarning("Cannot create git tag as GITHUB_CRED env var does not exist")
        return
    }
    
    if ($Context.IsDryRun) {
        Write-Output "DRY RUN: Would create git tag for server: $tag"
    } else {
        git config user.name vmp-ci
        git config user.email pr@vmp.ir
    
        git tag -a $tag $env:CI_COMMIT_SHA -m "${env:CI_COMMIT_REF_NAME}_$tag"
    
        git remote add github_tag https://$env:GITHUB_CRED@github.com/v-mp/vmp
        git push github_tag $tag
        git remote remove github_tag
    }
}