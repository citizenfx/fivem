# from http://stackoverflow.com/questions/2124753/how-i-can-use-powershell-with-the-visual-studio-command-prompt
function Invoke-BatchFile
{
	param([string]$Path)

	$tempFile = [IO.Path]::GetTempFileName()

	## Store the output of cmd.exe.  We also ask cmd.exe to output
	## the environment table after the batch file completesecho
	cmd.exe /c " `"$Path`" && set > `"$tempFile`" "

	## Go through the environment variables in the temp file.
	## For each of them, set the variable in our local environment.
	Get-Content $tempFile | Foreach-Object {
		if ($_ -match "^(.*?)=(.*)$")
		{
			Set-Content "env:\$($matches[1])" $matches[2]
		}
	}

	Remove-Item $tempFile
}
	

function Invoke-VSInit {
	$VCDir = (& "$PSScriptRoot\..\ci\vswhere.exe" -latest -property installationPath -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64)

	if (!(Test-Path Env:\DevEnvDir)) {
		Invoke-BatchFile "$VCDir\VC\Auxiliary\Build\vcvars64.bat"
	}

	if (!(Test-Path Env:\DevEnvDir)) {
		throw "No VC path!"
	}
}
