using module .\cfxBuildContext.psm1

function Invoke-SetupSubmodules {
    param(
        [CfxBuildContext] $Context
    )

    Push-Location $Context.ProjectRoot
        git submodule init | Out-Null
        git submodule sync | Out-Null

        $SubModules = git submodule | ForEach-Object {
            New-Object PSObject -Property @{
                Hash = $_.Substring(1).Split(' ')[0]
                Name = $_.Substring(1).Split(' ')[1]
            }
        }

        foreach ($submodule in $SubModules) {
            $SubmodulePath = git config -f .gitmodules --get "submodule.$($submodule.Name).path"

            if ((Test-Path $SubmodulePath) -and (Get-ChildItem $SubmodulePath).Length -gt 0) {
                continue;
            }
            
            $SubmoduleRemote = git config -f .gitmodules --get "submodule.$($submodule.Name).url"

            $Tag = (git ls-remote --tags $SubmoduleRemote | Select-String -Pattern $submodule.Hash | Select-Object -First 1) -replace '^.*tags/([^^]+).*$', '$1'

            if (!$Tag) {
                git clone $SubmoduleRemote $SubmodulePath
            } else {
                git clone -b $Tag --depth 1 --single-branch $SubmoduleRemote $SubmodulePath
            }
        }

        git submodule update --jobs=8 --force
    Pop-Location
}