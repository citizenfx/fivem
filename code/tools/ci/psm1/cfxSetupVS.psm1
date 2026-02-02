using module .\cfxBuildTools.psm1
using module .\cfxBuildContext.psm1

function Invoke-CfxSetupVS {
    param(
        [CfxBuildTools] $Tools,
        [CfxBuildContext] $Context
    )

    $VSDir = (& $Tools.vswhere -latest -prerelease -property installationPath     -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64)
    $VSVersionString = (& $Tools.vswhere -latest -prerelease -property catalog_buildVersion -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64)

    $Context.VSDir = $VSDir

    $VSFullVersion = [System.Version]::Parse($VSVersionString)
    
    if ($VSFullVersion -ge [System.Version]::Parse("18.0")) {
        $Context.PremakeVSVersion = "vs2026"
    }
    elseif ($VSFullVersion -ge [System.Version]::Parse("17.0")) {
        $Context.PremakeVSVersion = "vs2022"
    }
    elseif ($VSFullVersion -ge [System.Version]::Parse("16.0")) {
        $Context.PremakeVSVersion = "vs2019"
    }
    else {
        throw "Unknown or invalid VS version: $VSFullVersion."
    }

    if (!(Test-Path Env:\DevEnvDir)) {
        $VCVars64Path = "$VSDir\VC\Auxiliary\Build\vcvars64.bat"

        # from http://stackoverflow.com/questions/2124753/how-i-can-use-powershell-with-the-visual-studio-command-prompt
        $tempFile = [IO.Path]::GetTempFileName()

        ## Store the output of cmd.exe.  We also ask cmd.exe to output
        ## the environment table after the batch file completes
        cmd.exe /c " `"$VCVars64Path`" && set > `"$tempFile`" "

        ## Go through the environment variables in the temp file.
        ## For each of them, set the variable in our local environment.
        Get-Content $tempFile | Foreach-Object {
            if ($_ -match "^(.*?)=(.*)$") {
                Set-Content "env:\$($matches[1])" $matches[2]
            }
        }

        Remove-Item $tempFile
    }

    if (!(Test-Path Env:\DevEnvDir)) {
        throw "No Visual Studio DevEnvDir env var!"
    }

    if (!(Test-Path Env:\VSINSTALLDIR)) {
        throw "No Visual Studio VSINSTALLDIR env var!"
    }
}