param (
	#[Parameter(Mandatory=$true)]
	[string]
	$WorkDir = "C:\f\work",

	#[Parameter(Mandatory=$true)]
	[string]
	$SaveDir = "C:\f\save",

	[string]
	$GitRepo = "git@git.internal.fivem.net:cfx/cfx-client.git",

	[string]
	$Branch = "master",

	[bool]
	$DontUpload = $false,

	[bool]
	$DontBuild = $false,

	[string]
	$Identity = "C:\guava_deploy.ppk"
)

$CefName = "cef_binary_103.0.0-cfx-m103.2604+g164c280+chromium-103.0.5060.141_windows64_minimal"

Import-Module $PSScriptRoot\cache_build.psm1

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

function Start-Section
{
	param([Parameter(Position=0)][string]$Id, [Parameter(Position=1)][string]$Name)
	
	$ts = (Get-Date -UFormat %s -Millisecond 0)
	$esc = $([char]27)
	Write-Host "section_start:$($ts):$Id`r$esc[0K$Name"
}

function End-Section
{
	param([Parameter(Position=0)][string]$Id)
	
	$ts = (Get-Date -UFormat %s -Millisecond 0)
	$esc = $([char]27)
	Write-Host "section_end:$($ts):$Id`r$esc[0K"
}

$UploadBranch = "canary"
$TargetGame = "fivem"
$UploadType = "client"
$SentryProjectList = @("fivem-client-1604")
$SentryVersion = "cfx-${env:CI_PIPELINE_ID}"

if ($env:IS_FXSERVER -eq 1) {
	$TargetGame = "fxserver"
	$UploadType = "server"
	$SentryProjectList = @("fxserver")
} elseif ($env:IS_RDR3 -eq 1) {
	$TargetGame = "redm"
	$UploadType = "rdr3"
	$SentryProjectList = @("redm")
}

if ($env:CI) {
	if ($env:APPVEYOR) {
		$Branch = $env:APPVEYOR_REPO_BRANCH
		$WorkDir = $env:APPVEYOR_BUILD_FOLDER -replace '/','\'

		$UploadBranch = $env:APPVEYOR_REPO_BRANCH

		$Tag = "vUndefined"
	} else {
		$Branch = $env:CI_COMMIT_REF_NAME
		$WorkDir = $env:CI_PROJECT_DIR -replace '/','\'

		$UploadBranch = $env:CI_COMMIT_REF_NAME

		if ($TargetGame -eq "fxserver") {
			$Tag = "v1.0.0.${env:CI_PIPELINE_ID}"

			git config user.name citizenfx-ci
			git config user.email pr@fivem.net
			git tag -a $Tag $env:CI_COMMIT_SHA -m "${env:CI_COMMIT_REF_NAME}_$Tag"
			git remote add github_tag https://$env:GITHUB_CRED@github.com/citizenfx/fivem.git
			git push github_tag $Tag
			git remote remove github_tag

			$GlobalTag = $Tag
			$SentryVersion = $GlobalTag
		}
	}

	if ($TargetGame -eq "fxserver") {
		$UploadBranch += " SERVER"
	} elseif ($TargetGame -eq "redm") {
		$UploadBranch += " RDR3"
	}
}

$WorkRootDir = "$WorkDir\code\"

$BinRoot = "$SaveDir\bin\$UploadType\$Branch\" -replace '/','\'
$BuildRoot = "$SaveDir\build\$UploadType\$Branch\" -replace '/', '\'

$env:TargetPlatformVersion = "10.0.15063.0"

Add-Type -A 'System.IO.Compression.FileSystem'

New-Item -ItemType Directory -Force $SaveDir | Out-Null
New-Item -ItemType Directory -Force $WorkDir | Out-Null
New-Item -ItemType Directory -Force $BinRoot | Out-Null
New-Item -ItemType Directory -Force $BuildRoot | Out-Null

Set-Location $WorkRootDir

if ($null -eq (Get-Command "python.exe" -ErrorAction SilentlyContinue)) {
	$env:Path = "C:\python27\;" + $env:Path
}

if (!($env:BOOST_ROOT)) {
	if (Test-Path C:\Libraries\boost_1_71_0) {
		$env:BOOST_ROOT = "C:\Libraries\boost_1_71_0"
	} else {
		$env:BOOST_ROOT = "C:\dev\boost_1_71_0"
	}
}

Push-Location $WorkDir
$GameVersion = ((git rev-list HEAD | measure-object).Count * 10) + 1100000

$LauncherCommit = (git rev-list -1 HEAD code/client/launcher/ code/shared/ code/client/shared/ code/tools/dbg/ vendor/breakpad/ vendor/tinyxml2/ vendor/xz/ vendor/curl/ vendor/cpr/ vendor/minizip/ code/premake5.lua)
$LauncherVersion = ((git rev-list $LauncherCommit | measure-object).Count * 10) + 1100000

$SDKCommit = (git rev-list -1 HEAD ext/sdk-build/ ext/sdk/ code/tools/ci/build_sdk.ps1)
$SDKVersion = ((git rev-list $SDKCommit | measure-object).Count * 10) + 1100000
Pop-Location

if (!$DontBuild)
{
	Start-Section "vs_setup" "Setting up VS"

	$VCDir = (& "$WorkDir\code\tools\ci\vswhere.exe" -latest -prerelease -property installationPath -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64)

	$VSVersion = [System.Version]::Parse((& "$WorkDir\code\tools\ci\vswhere.exe" -prerelease -latest -property catalog_buildVersion -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64))
	if ($VSVersion -ge [System.Version]::Parse("17.0")) {
		$VSLine = "vs2022"
	} elseif ($VSVersion -ge [System.Version]::Parse("16.0")) {
		$VSLine = "vs2019"
	} else {
		throw "Unknown or invalid VS version."
	}

	if (!(Test-Path Env:\DevEnvDir)) {
		Invoke-BatchFile "$VCDir\VC\Auxiliary\Build\vcvars64.bat"
	}

	if (!(Test-Path Env:\DevEnvDir)) {
		throw "No VC path!"
	}

	End-Section "vs_setup"

	Start-Section "update_submodules" "Updating submodules"
	Push-Location $WorkDir

	git submodule init
	git submodule sync

	Push-Location $WorkDir
	$SubModules = git submodule | ForEach-Object { New-Object PSObject -Property @{ Hash = $_.Substring(1).Split(' ')[0]; Name = $_.Substring(1).Split(' ')[1] } }

	foreach ($submodule in $SubModules) {
		$SubmodulePath = git config -f .gitmodules --get "submodule.$($submodule.Name).path"

		if ((Test-Path $SubmodulePath) -and (Get-ChildItem $SubmodulePath).Length -gt 0) {
			continue;
		}
		
		Start-Section "update_submodule_$($submodule.Name)" "Cloning $($submodule.Name)"
		$SubmoduleRemote = git config -f .gitmodules --get "submodule.$($submodule.Name).url"

		$Tag = (git ls-remote --tags $SubmoduleRemote | Select-String -Pattern $submodule.Hash | Select-Object -First 1) -replace '^.*tags/([^^]+).*$','$1'

		if (!$Tag) {
			git clone $SubmoduleRemote $SubmodulePath
		} else {
			git clone -b $Tag --depth 1 --single-branch $SubmoduleRemote $SubmodulePath
		}

		End-Section "update_submodule_$($submodule.Name)"
	}
	Pop-Location

	Start-Section "update_submodule_git" "Updating all submodules"
	git submodule update --jobs=8 --force
	End-Section "update_submodule_git"

	Pop-Location

	End-Section "update_submodules"

	Start-Section "run_prebuild" "Running prebuild"
	Push-Location $WorkDir
	.\prebuild.cmd
	Pop-Location
	End-Section "run_prebuild"

	if ($TargetGame -ne "fxserver") {
		Start-Section "dl_chrome" "Downloading Chrome"
		try {
			if (!(Test-Path "$SaveDir\$CefName.zip")) {
				curl.exe -Lo "$SaveDir\$CefName.zip" "https://runtime.fivem.net/build/cef/$CefName.zip"
			}

			tar.exe -C $WorkDir\vendor\cef -xf "$SaveDir\$CefName.zip"
			Move-Item -Force $WorkDir\vendor\cef\$CefName\* $WorkDir\vendor\cef\
			Remove-Item -Recurse $WorkDir\vendor\cef\$CefName\
		} catch {
			return
		}
		End-Section "dl_chrome"
	}

	Start-Section "build" "Building"

	if ($env:FIVEM_PRIVATE_URI) {
		Start-Section "private" "Fetching privates"
		Push-Location $WorkDir\..\
		
		$CIBranch = "master"

		# cloned, building
		if (!(Test-Path fivem-private)) {
			git clone -b $CIBranch $env:FIVEM_PRIVATE_URI
		} else {
			Set-Location fivem-private

			git fetch origin | Out-Null
			git reset --hard origin/$CIBranch | Out-Null

			Set-Location ..
		}

		Write-Output "private_repo '../../fivem-private/'" | Out-File -Encoding ascii $WorkRootDir\privates_config.lua

		Pop-Location
		End-Section "private"
	}

	$GameName = "five"
	$BuildPath = "$BuildRoot\five"

	if ($TargetGame -eq "fxserver") {
		$GameName = "server"
		$BuildPath = "$BuildRoot\server\windows"
	} elseif ($TargetGame -eq "redm") {
		$GameName = "rdr3"
		$BuildPath = "$BuildRoot\rdr3"
	}

	Start-Section "premake" "Running premake"
	Invoke-Expression "& $WorkRootDir\tools\ci\premake5 $VSLine --game=$GameName --builddir=$BuildRoot --bindir=$BinRoot"
	End-Section "premake"

	"#pragma once
	#define BASE_EXE_VERSION $LauncherVersion" | Out-File -Force shared\citversion.h.tmp

	if ((!(Test-Path shared\citversion.h)) -or ($null -ne (Compare-Object (Get-Content shared\citversion.h.tmp) (Get-Content shared\citversion.h)))) {
		Remove-Item -Force shared\citversion.h
		Move-Item -Force shared\citversion.h.tmp shared\citversion.h

		if (Test-Path env:\CI_PIPELINE_ID) {
			"#pragma once
			#define EXE_VERSION ${env:CI_PIPELINE_ID}
" | Out-File -Force shared\launcher_version.h
		}
	}

	"#pragma once
	#define GIT_DESCRIPTION ""$UploadBranch $GlobalTag win32""
	#define GIT_TAG ""$GlobalTag""" | Out-File -Force shared\cfx_version.h

	remove-item env:\platform
	$env:UseMultiToolTask = "true"
	$env:EnforceProcessCountAcrossBuilds = "true"

	# restore nuget packages
	Start-Section "nuget" "Running nuget.exe"
	Invoke-Expression "& $WorkRootDir\tools\ci\nuget.exe restore $BuildPath\CitizenMP.sln"
	End-Section "nuget"

	#echo $env:Path
	#/logger:C:\f\customlogger.dll /noconsolelogger
	Start-Section "msbuild" "Running msbuild..."
	msbuild /p:preferredtoolarchitecture=x64 /p:configuration=release /v:q /m $BuildPath\CitizenMP.sln

	if (!$?) {
		throw "Failed to build the code."
	}

	End-Section "msbuild"

	if ((($env:COMPUTERNAME -eq "AVALON2") -or ($env:COMPUTERNAME -eq "AVALON") -or ($env:COMPUTERNAME -eq "OMNITRON")) -and ($TargetGame -ne "fxserver")) {
		Start-Process -NoNewWindow powershell -ArgumentList "-ExecutionPolicy unrestricted .\tools\ci\dump_symbols.ps1 -BinRoot $BinRoot -GameName $GameName"
	} elseif (($TargetGame -eq "fxserver") -and (Test-Path C:\h\debuggers)) {
		Start-Process -NoNewWindow powershell -ArgumentList "-ExecutionPolicy unrestricted .\tools\ci\dump_symbols_server.ps1 -BinRoot $BinRoot"
	}

	End-Section "build"
}

Set-Location $WorkRootDir

if (!$DontBuild -and ($TargetGame -eq "fxserver")) {
	Remove-Item -Recurse -Force $WorkDir\out
	
	Start-Section "sr" "Building system resources"
	# build UI
	Push-Location $WorkDir
	$SRCommit = (git rev-list -1 HEAD ext/txAdmin ext/system-resources/)
	Pop-Location

	Push-Location $WorkDir\ext\system-resources

	$SRSucceeded = $true

	if ($SRCommit -ne (Get-Content .commit)) {
		.\build.cmd
		$SRSucceeded = $?
		
		$SRCommit | Out-File -Encoding ascii -NoNewline .commit
	}

	# same as for UISucceeded
	if ($SRSucceeded -or $env:APPVEYOR) {
		Remove-Item -Recurse -Force $WorkDir\data\server\citizen\system_resources\ | Out-Null
		New-Item -ItemType Directory -Force $WorkDir\data\server\citizen\system_resources\ | Out-Null
		Copy-Item -Force -Recurse $WorkDir\ext\system-resources\data\* $WorkDir\data\server\citizen\system_resources\
	} else {
		throw "Failed to build system resources"
	}

	Pop-Location
	End-Section "sr"

	Remove-Item -Recurse -Force $WorkDir\out | Out-Null
	New-Item -ItemType Directory -Force $WorkDir\out | Out-Null
	New-Item -ItemType Directory -Force $WorkDir\out\server | Out-Null
	New-Item -ItemType Directory -Force $WorkDir\out\server\citizen | Out-Null

	Copy-Item -Force $BinRoot\server\windows\release\*.exe $WorkDir\out\server\
	Copy-Item -Force $BinRoot\server\windows\release\*.dll $WorkDir\out\server\

	Copy-Item -Force -Recurse $BinRoot\server\windows\release\citizen\* $WorkDir\out\server\citizen\

	Copy-Item -Force -Recurse $WorkDir\data\shared\* $WorkDir\out\server\
	Copy-Item -Force -Recurse $WorkDir\data\redist\crt\* $WorkDir\out\server\
	Copy-Item -Force -Recurse $WorkDir\data\server\* $WorkDir\out\server\
	Copy-Item -Force -Recurse $WorkDir\data\server_windows\* $WorkDir\out\server\

	Remove-Item -Force $WorkDir\out\server\citizen\.gitignore

	# breaks downlevel OS compat
	Remove-Item -Force $WorkDir\out\server\dbghelp.dll
	
	# useless client-related scripting stuff
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\lua\natives_0*.zip
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\lua\natives_2*.zip
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\lua\*_universal.lua
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\lua\*_universal.zip
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\lua\natives_0*.lua
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\lua\natives_2*.lua
	
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\v8\*_universal.d.ts
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\v8\*_universal.js
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\v8\natives_0*.*
	Remove-Item -Force $WorkDir\out\server\citizen\scripting\v8\natives_2*.*
	
	Copy-Item -Force "$WorkRootDir\tools\ci\7z.exe" 7z.exe

	.\7z.exe a -mx=9 $WorkDir\out\server.zip $WorkDir\out\server\*
	.\7z.exe a -mx=7 $WorkDir\out\server.7z $WorkDir\out\server\*
}

$CacheDir = "$SaveDir\caches\$Branch"

if ($TargetGame -eq "redm") {
	$CacheDir = "$SaveDir\rcaches\$Branch"
}

if (!$DontBuild -and ($TargetGame -ne "fxserver")) {
	# prepare caches
	New-Item -ItemType Directory -Force $CacheDir | Out-Null
	New-Item -ItemType Directory -Force $CacheDir\fivereborn | Out-Null
	New-Item -ItemType Directory -Force $CacheDir\fivereborn\citizen | Out-Null
	Set-Location $CacheDir

	if ($true) {
		Start-Section "ui" "Building UI"
		# build UI
		Push-Location $WorkDir
		$UICommit = (git rev-list -1 HEAD ext/ui-build/ ext/cfx-ui/)
		Pop-Location

		Push-Location $WorkDir\ext\ui-build

		$UiSucceeded = $true

		if ($UICommit -ne (Get-Content data\.commit)) {
			.\build.cmd
			$UiSucceeded = $?
			
			$UICommit | Out-File -Encoding ascii -NoNewline data\.commit
		}

		# appveyor somehow fails the $? check
		if ($UiSucceeded -or $env:APPVEYOR) {
			Copy-Item -Force $WorkDir\ext\ui-build\data.zip $CacheDir\fivereborn\citizen\ui.zip
			Copy-Item -Force $WorkDir\ext\ui-build\data_big.zip $CacheDir\fivereborn\citizen\ui-big.zip
		} else {
			throw "Failed to build UI"
		}

		Pop-Location
		End-Section "ui"
	}
	
	Start-Section "caches" "Gathering caches"
	Remove-Item -Force $CacheDir\fivereborn\citizen\re3.rpf

	# copy output files
	New-Item -ItemType Directory -Force $CacheDir\fivereborn\bin

	Copy-Item -Force -Recurse $WorkDir\vendor\cef\Release\*.dll $CacheDir\fivereborn\bin\
	Copy-Item -Force -Recurse $WorkDir\vendor\cef\Release\*.bin $CacheDir\fivereborn\bin\

	New-Item -ItemType Directory -Force $CacheDir\fivereborn\bin\cef

	Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\icudtl.dat $CacheDir\fivereborn\bin\
	Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\*.pak $CacheDir\fivereborn\bin\cef\
	Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\locales\en-US.pak $CacheDir\fivereborn\bin\cef\

	# remove CEF as redownloading is broken and this slows down gitlab ci cache
	Remove-Item -Recurse $WorkDir\vendor\cef\*

	if ($TargetGame -eq "fivem") {
		Copy-Item -Force -Recurse $WorkDir\data\shared\* $CacheDir\fivereborn\
		Copy-Item -Force -Recurse $WorkDir\data\client\* $CacheDir\fivereborn\
		Copy-Item -Force -Recurse $WorkDir\data\redist\crt\* $CacheDir\fivereborn\bin\
		
		Remove-Item -Force -Recurse $CacheDir\fivereborn\grpc-ipfs.dll
		Remove-Item -Force -Recurse $CacheDir\fivereborn\ipfsdl.dll
	} elseif ($TargetGame -eq "redm") {
		Copy-Item -Force -Recurse $WorkDir\data\shared\* $CacheDir\fivereborn\
		Copy-Item -Force -Recurse $WorkDir\data\client\*.dll $CacheDir\fivereborn\
		Copy-Item -Force -Recurse $WorkDir\data\client\bin\* $CacheDir\fivereborn\bin\
		Copy-Item -Force -Recurse $WorkDir\data\redist\crt\* $CacheDir\fivereborn\bin\
		Copy-Item -Force -Recurse $WorkDir\data\client\citizen\clr2 $CacheDir\fivereborn\citizen\
		Copy-Item -Force -Recurse $WorkDir\data\client\citizen\*.ttf $CacheDir\fivereborn\citizen\
		Copy-Item -Force -Recurse $WorkDir\data\client\citizen\ros $CacheDir\fivereborn\citizen\
		Copy-Item -Force -Recurse $WorkDir\data\client\citizen\resources $CacheDir\fivereborn\citizen\
		Copy-Item -Force -Recurse $WorkDir\data\client_rdr\* $CacheDir\fivereborn\
		
		Copy-Item -Force -Recurse C:\f\grpc-ipfs.dll $CacheDir\fivereborn\
	}
	
	if ($TargetGame -eq "fivem") {
		Copy-Item -Force $BinRoot\five\release\*.dll $CacheDir\fivereborn\
		Copy-Item -Force $BinRoot\five\release\*.com $CacheDir\fivereborn\
		Copy-Item -Force $BinRoot\five\release\CitizenFX_SubProcess_*.bin $CacheDir\fivereborn\

		Copy-Item -Force $BinRoot\five\release\FiveM_Diag.exe $CacheDir\fivereborn\
		Copy-Item -Force -Recurse $BinRoot\five\release\citizen\* $CacheDir\fivereborn\citizen\
	} elseif ($TargetGame -eq "redm") {
		Copy-Item -Force $BinRoot\rdr3\release\*.dll $CacheDir\fivereborn\
		Copy-Item -Force $BinRoot\rdr3\release\*.com $CacheDir\fivereborn\
		Copy-Item -Force $BinRoot\rdr3\release\CitizenFX_SubProcess_*.bin $CacheDir\fivereborn\

		Copy-Item -Force -Recurse $BinRoot\rdr3\release\citizen\* $CacheDir\fivereborn\citizen\
	}
	
	"$GameVersion" | Out-File -Encoding ascii $CacheDir\fivereborn\citizen\version.txt
	"${env:CI_PIPELINE_ID}" | Out-File -Encoding ascii $CacheDir\fivereborn\citizen\release.txt
	
	if (($env:COMPUTERNAME -eq "AVALON") -or ($env:COMPUTERNAME -eq "OMNITRON") -or ($env:COMPUTERNAME -eq "AVALON2")) {
		Push-Location C:\f\bci\
		.\BuildComplianceInfo.exe $CacheDir\fivereborn\ C:\f\bci-list.txt
		Pop-Location
	}

	# build meta/xz variants
	"<Caches>
		<Cache ID=`"fivereborn`" Version=`"$GameVersion`" />
	</Caches>" | Out-File -Encoding ascii $CacheDir\caches.xml

	End-Section "caches"

	Copy-Item -Force "$WorkRootDir\tools\ci\xz.exe" xz.exe

	# build bootstrap executable
	if ($TargetGame -eq "fivem") {
		Copy-Item -Force $BinRoot\five\release\FiveM.exe CitizenFX.exe
	} elseif ($TargetGame -eq "redm") {
		Copy-Item -Force $BinRoot\rdr3\release\CitiLaunch.exe CitizenFX.exe
	}

	if (Test-Path CitizenFX.exe.xz) {
		Remove-Item CitizenFX.exe.xz
	}

	Start-Section "caches_fin" "Gathering more caches"
	Invoke-Expression "& $WorkRootDir\tools\ci\xz.exe -9 CitizenFX.exe"

	$LauncherLength = (Get-ItemProperty CitizenFX.exe.xz).Length
	"$LauncherVersion $LauncherLength" | Out-File -Encoding ascii version.txt

	# build bootstrap executable
	if ($TargetGame -eq "fivem") {
		Copy-Item -Force $BinRoot\five\release\FiveM.exe $CacheDir\fivereborn\CitizenFX.exe
	} elseif ($TargetGame -eq "redm") {
		Copy-Item -Force $BinRoot\rdr3\release\CitiLaunch.exe $CacheDir\fivereborn\CitizenFX.exe
	}

	Remove-Item -Recurse -Force $WorkDir\caches
	Copy-Item -Recurse -Force $CacheDir $WorkDir\caches
	End-Section "caches_fin"
}

if (!$DontBuild) {
	$uri = 'https://sentry.fivem.net/api/0/organizations/citizenfx/releases/'
	$json = @{
		version = "$SentryVersion"
		refs = @(
			@{
				repository = 'citizenfx/fivem'
				commit = $env:CI_COMMIT_SHA
			}
		)
		projects = @("fxserver")
	} | ConvertTo-Json

	$headers = New-Object "System.Collections.Generic.Dictionary[[String],[String]]"
	$headers.Add('Authorization', "Bearer $env:SENTRY_TOKEN")

	Invoke-RestMethod -Uri $uri -Method Post -Headers $headers -Body $json -ContentType 'application/json'
}

if (!$DontUpload) {
	Remove-Item -Recurse -Force $CacheDir
	Copy-Item -Recurse -Force $WorkDir\caches $CacheDir

	$UploadBranch = $env:CI_ENVIRONMENT_NAME

	$CacheName = "eh"

	if ($TargetGame -eq "fivem") {
		$CacheName = "fivereborn"
	} elseif ($TargetGame -eq "redm") {
		$CacheName = "redm"
	}

	# for xz.exe
	$env:PATH += ";$WorkRootDir\tools\ci"

	Remove-Item -Force $CacheDir\fivereborn\info.xml
	Invoke-CacheGen -Source $CacheDir\fivereborn -CacheName $CacheName -BranchName $UploadBranch -BranchVersion $GameVersion -BootstrapName CitizenFX.exe -BootstrapVersion $LauncherVersion

	Set-Location $CacheDir

	$Branch = $UploadBranch

	$env:Path = "C:\msys64\usr\bin;$env:Path"

	if ($TargetGame -eq "fivem") {
		Remove-Item -Force $WorkDir\caches\fxdk-five\info.xml
		Invoke-CacheGen -Source $WorkDir\caches\fxdk-five -CacheName "fxdk-five" -BranchName $UploadBranch -BranchVersion $SDKVersion -BootstrapName CitizenFX.exe -BootstrapVersion $LauncherVersion
	}

	$uri = "https://sentry.fivem.net/api/0/organizations/citizenfx/releases/${SentryVersion}/deploys/"
	$json = @{
		environment = $UploadBranch
		projects = $SentryProjectList
	} | ConvertTo-Json

	$headers = New-Object "System.Collections.Generic.Dictionary[[String],[String]]"
	$headers.Add('Authorization', "Bearer $env:SENTRY_TOKEN")

	Invoke-RestMethod -Uri $uri -Method Post -Headers $headers -Body $json -ContentType 'application/json'

	Set-Location (Split-Path -Parent $WorkDir)

	[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
	Invoke-WebRequest -UseBasicParsing -Uri $env:REFRESH_URL -Method GET | out-null
}
