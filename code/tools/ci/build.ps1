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

$CefName = "cef_binary_73.0.0-cef-patchset.1936+ga086e57+chromium-73.0.3683.75_windows64_minimal"

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

function Invoke-WebHook
{
    param([string]$Text)

    $payload = @{
	    "text" = $Text;
    }

    if (!$env:TG_WEBHOOK)
    {
        return
    }

    iwr -UseBasicParsing -Uri $env:TG_WEBHOOK -Method POST -Body (ConvertTo-Json -Compress -InputObject $payload) | out-null

    $payload.text += " <:mascot:295575900446130176>"#<@&297070674898321408>"

    iwr -UseBasicParsing -Uri $env:DISCORD_WEBHOOK -Method POST -Body (ConvertTo-Json -Compress -InputObject $payload) | out-null
}

$inCI = $false
$Triggerer = "$env:USERDOMAIN\$env:USERNAME"
$UploadBranch = "canary"
$IsServer = $false
$IsLauncher = $false
$IsRDR = $false
$UploadType = "client"

if ($env:IS_FXSERVER -eq 1) {
    $IsServer = $true
    $UploadType = "server"
} elseif ($env:IS_LAUNCHER -eq 1) {
    $IsLauncher = $true
    $UploadType = "launcher"
} elseif ($env:IS_RDR3 -eq 1) {
    $IsRDR = $true
    $UploadType = "rdr3"
}

if ($env:CI) {
    $inCI = $true

    if ($env:APPVEYOR) {
    	$Branch = $env:APPVEYOR_REPO_BRANCH
    	$WorkDir = $env:APPVEYOR_BUILD_FOLDER -replace '/','\'

    	$Triggerer = $env:APPVEYOR_REPO_COMMIT_AUTHOR_EMAIL

    	$UploadBranch = $env:APPVEYOR_REPO_BRANCH

        $Tag = "vUndefined"
    } else {
    	$Branch = $env:CI_BUILD_REF_NAME
    	$WorkDir = $env:CI_PROJECT_DIR -replace '/','\'

    	$Triggerer = $env:GITLAB_USER_EMAIL

    	$UploadBranch = $env:CI_COMMIT_REF_NAME

    	if ($IsServer) {
            $Tag = "v1.0.0.${env:CI_PIPELINE_ID}"

            git config user.name citizenfx-ci
            git config user.email pr@fivem.net
    		git tag -a $Tag $env:CI_COMMIT_SHA -m "${env:CI_COMMIT_REF_NAME}_$Tag"
            git remote add github_tag https://$env:GITHUB_CRED@github.com/citizenfx/fivem.git
            git push github_tag $Tag
            git remote remove github_tag

            $GlobalTag = $Tag
    	}
    }

    if ($IsServer) {
        $UploadBranch += " SERVER"
    } elseif ($IsLauncher) {
        $UploadBranch += " COMPOSITOR"
    } elseif ($IsRDR) {
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

if ((Get-Command "python.exe" -ErrorAction SilentlyContinue) -eq $null) {
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
Pop-Location

if (!$DontBuild)
{
    Invoke-WebHook "Bloop, building a new $env:CI_PROJECT_NAME $UploadBranch build, triggered by $Triggerer"

    Write-Host "[checking if repository is latest version]" -ForegroundColor DarkMagenta

    $ci_dir = $env:CI_PROJECT_DIR -replace '/','\'

    #cmd /c mklink /d citizenmp cfx-client

    $VCDir = (& "$WorkDir\code\tools\ci\vswhere.exe" -latest -prerelease -property installationPath -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64)

    if (!(Test-Path Env:\DevEnvDir)) {
        Invoke-BatchFile "$VCDir\VC\Auxiliary\Build\vcvars64.bat"
    }

    if (!(Test-Path Env:\DevEnvDir)) {
        throw "No VC path!"
    }

    Write-Host "[updating submodules]" -ForegroundColor DarkMagenta
    Push-Location $WorkDir

    git submodule init

    $SubModules = git submodule | ForEach-Object { New-Object PSObject -Property @{ Hash = $_.Substring(1).Split(' ')[0]; Name = $_.Substring(1).Split(' ')[1] } }

    foreach ($submodule in $SubModules) {
        $SubmodulePath = git config -f .gitmodules --get "submodule.$($submodule.Name).path"

        if (Test-Path $SubmodulePath) {
			continue;
		}
		
        $SubmoduleRemote = git config -f .gitmodules --get "submodule.$($submodule.Name).url"

        $Tag = (git ls-remote --tags $SubmoduleRemote | Select-String -Pattern $submodule.Hash) -replace '^.*tags/([^^]+).*$','$1'

        if (!$Tag) {
            git clone $SubmoduleRemote $SubmodulePath
        } else {
            git clone -b $Tag --depth 1 --single-branch $SubmoduleRemote $SubmodulePath
        }
    }

    git submodule update

    Pop-Location

    Write-Host "[running prebuild]" -ForegroundColor DarkMagenta
    Push-Location $WorkDir
    .\prebuild.cmd
    Pop-Location

    if (!$IsServer) {
        Write-Host "[downloading chrome]" -ForegroundColor DarkMagenta
        try {
            if (!(Test-Path "$SaveDir\$CefName.zip")) {
                Invoke-WebRequest -UseBasicParsing -OutFile "$SaveDir\$CefName.zip" "https://runtime.fivem.net/build/cef/$CefName.zip"
            }

            Expand-Archive -Force -Path "$SaveDir\$CefName.zip" -DestinationPath $WorkDir\vendor\cef
            Move-Item -Force $WorkDir\vendor\cef\$CefName\* $WorkDir\vendor\cef\
            Remove-Item -Recurse $WorkDir\vendor\cef\$CefName\
        } catch {
            return
        }
    }

    Write-Host "[building]" -ForegroundColor DarkMagenta

	if (!($env:APPVEYOR)) {
	    Push-Location $WorkDir\..\

	    # cloned, building
	    if (!(Test-Path fivem-private)) {
	        git clone $env:FIVEM_PRIVATE_URI
	    } else {
	        cd fivem-private

	        git fetch origin | Out-Null
	        git reset --hard origin/master | Out-Null

	        cd ..
	    }

	    echo "private_repo '../../fivem-private/'" | Out-File -Encoding ascii $WorkRootDir\privates_config.lua

	    Pop-Location
	}

    $GameName = "five"
    $BuildPath = "$BuildRoot\five"

    if ($IsServer) {
        $GameName = "server"
        $BuildPath = "$BuildRoot\server\windows"
    } elseif ($IsLauncher) {
        $GameName = "launcher"
        $BuildPath = "$BuildRoot\launcher"
    } elseif ($IsRDR) {
        $GameName = "rdr3"
        $BuildPath = "$BuildRoot\rdr3"
    }

    Invoke-Expression "& $WorkRootDir\tools\ci\premake5 vs2019 --game=$GameName --builddir=$BuildRoot --bindir=$BinRoot"

    "#pragma once
    #define BASE_EXE_VERSION $LauncherVersion" | Out-File -Force shared\citversion.h.tmp

    if ((!(Test-Path shared\citversion.h)) -or ($null -ne (Compare-Object (Get-Content shared\citversion.h.tmp) (Get-Content shared\citversion.h)))) {
        Remove-Item -Force shared\citversion.h
        Move-Item -Force shared\citversion.h.tmp shared\citversion.h
    }

    "#pragma once
    #define GIT_DESCRIPTION ""$UploadBranch $GlobalTag win32""
    #define GIT_TAG ""$GlobalTag""" | Out-File -Force shared\cfx_version.h

    remove-item env:\platform

	# restore nuget packages
	Invoke-Expression "& $WorkRootDir\tools\ci\nuget.exe restore $BuildPath\CitizenMP.sln"

    #echo $env:Path
    #/logger:C:\f\customlogger.dll /noconsolelogger
    msbuild /p:preferredtoolarchitecture=x64 /p:configuration=release /v:q /fl /m:4 $BuildPath\CitizenMP.sln

    if (!$?) {
        Invoke-WebHook "Building FiveM failed :("
        throw "Failed to build the code."
    }

    if ((($env:COMPUTERNAME -eq "BUILDVM") -or ($env:COMPUTERNAME -eq "AVALON") -or ($env:COMPUTERNAME -eq "OMNITRON")) -and (!$IsServer)) {
        Start-Process -NoNewWindow powershell -ArgumentList "-ExecutionPolicy unrestricted .\tools\ci\dump_symbols.ps1 -BinRoot $BinRoot -GameName $GameName"
    } elseif ($IsServer -and (Test-Path C:\h\debuggers)) {
		Start-Process -NoNewWindow powershell -ArgumentList "-ExecutionPolicy unrestricted .\tools\ci\dump_symbols_server.ps1 -BinRoot $BinRoot"
    }
}

Set-Location $WorkRootDir

if (!$DontBuild -and $IsServer) {
    Remove-Item -Recurse -Force $WorkDir\out
    
    Push-Location $WorkDir\ext\system-resources
    .\build.cmd

    if ($?) {
        New-Item -ItemType Directory -Force $WorkDir\data\server\citizen\system_resources\ | Out-Null
        Copy-Item -Force -Recurse $WorkDir\ext\system-resources\data\* $WorkDir\data\server\citizen\system_resources\
    }

    Pop-Location

    New-Item -ItemType Directory -Force $WorkDir\out | Out-Null
    New-Item -ItemType Directory -Force $WorkDir\out\server | Out-Null
    New-Item -ItemType Directory -Force $WorkDir\out\server\citizen | Out-Null

    Copy-Item -Force $BinRoot\server\windows\release\*.exe $WorkDir\out\server\
    Copy-Item -Force $BinRoot\server\windows\release\*.dll $WorkDir\out\server\

    Copy-Item -Force -Recurse $BinRoot\server\windows\release\citizen\* $WorkDir\out\server\citizen\

    Copy-Item -Force -Recurse $WorkDir\data\shared\* $WorkDir\out\server\
    Copy-Item -Force -Recurse $WorkDir\data\client\v8* $WorkDir\out\server\
    Copy-Item -Force -Recurse $WorkDir\data\client\bin\icu* $WorkDir\out\server\
    Copy-Item -Force -Recurse $WorkDir\data\server\* $WorkDir\out\server\
    Copy-Item -Force -Recurse $WorkDir\data\server_windows\* $WorkDir\out\server\

    Remove-Item -Force $WorkDir\out\server\citizen\.gitignore

    Copy-Item -Force "$WorkRootDir\tools\ci\7z.exe" 7z.exe

    .\7z.exe a $WorkDir\out\server.zip $WorkDir\out\server\*

    $uri = 'https://sentry.fivem.net/api/0/organizations/citizenfx/releases/'
    $json = @{
    	version = "$GlobalTag"
    	refs = @(
    		@{
    			repository = 'citizenfx/fivem'
    			commit = $env:CI_COMMIT_SHA
    		}
    	)
    	projects = @("fxs")
    } | ConvertTo-Json

    $headers = New-Object "System.Collections.Generic.Dictionary[[String],[String]]"
    $headers.Add('Authorization', "Bearer $env:SENTRY_TOKEN")

    Invoke-RestMethod -Uri $uri -Method Post -Headers $headers -Body $json -ContentType 'application/json'

    Invoke-WebHook "Bloop, building a SERVER/WINDOWS build completed!"
}

$CacheDir = "$SaveDir\caches"

if ($IsLauncher) {
    $CacheDir = "$SaveDir\lcaches"
}

if ($IsRDR) {
    $CacheDir = "$SaveDir\rcaches"
}

if (!$DontBuild -and !$IsServer) {
    # prepare caches
    New-Item -ItemType Directory -Force $CacheDir | Out-Null
    New-Item -ItemType Directory -Force $CacheDir\fivereborn | Out-Null
    Set-Location $CacheDir

    if ($true) {
        # build UI
        Push-Location $WorkDir
        $UICommit = (git rev-list -1 HEAD ext/ui-build/ ext/cfx-ui/)
        Pop-Location

        Push-Location $WorkDir\ext\ui-build

        if ($UICommit -ne (Get-Content data\.commit)) {
            .\build.cmd
            
            $UICommit | Out-File -Encoding ascii -NoNewline data\.commit
        }

        if ($?) {
            Copy-Item -Force $WorkDir\ext\ui-build\data.zip $CacheDir\fivereborn\citizen\ui.zip
            Copy-Item -Force $WorkDir\ext\ui-build\data_big.zip $CacheDir\fivereborn\citizen\ui-big.zip
        }

        Pop-Location
    }

    # copy output files
    Copy-Item -Force -Recurse $WorkDir\vendor\cef\Release\*.dll $CacheDir\fivereborn\bin\
    Copy-Item -Force -Recurse $WorkDir\vendor\cef\Release\*.bin $CacheDir\fivereborn\bin\

    New-Item -ItemType Directory -Force $CacheDir\fivereborn\bin\cef

    Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\icudtl.dat $CacheDir\fivereborn\bin\
    Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\*.pak $CacheDir\fivereborn\bin\cef\
    Copy-Item -Force -Recurse $WorkDir\vendor\cef\Resources\locales\en-US.pak $CacheDir\fivereborn\bin\cef\

    # remove CEF as redownloading is broken and this slows down gitlab ci cache
    Remove-Item -Recurse $WorkDir\vendor\cef\*

    if (!$IsLauncher -and !$IsRDR) {
        Copy-Item -Force -Recurse $WorkDir\data\shared\* $CacheDir\fivereborn\
        Copy-Item -Force -Recurse $WorkDir\data\client\* $CacheDir\fivereborn\
        
        Copy-Item -Force -Recurse C:\f\grpc-ipfs.dll $CacheDir\fivereborn\
    } elseif ($IsLauncher) {
        Copy-Item -Force -Recurse $WorkDir\data\launcher\* $CacheDir\fivereborn\
        Copy-Item -Force -Recurse $WorkDir\data\client\bin\* $CacheDir\fivereborn\bin\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\resources\* $CacheDir\fivereborn\citizen\resources\
    } elseif ($IsRDR) {
        Copy-Item -Force -Recurse $WorkDir\data\shared\* $CacheDir\fivereborn\
        Copy-Item -Force -Recurse $WorkDir\data\client\*.dll $CacheDir\fivereborn\
        Copy-Item -Force -Recurse $WorkDir\data\client\bin\* $CacheDir\fivereborn\bin\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\clr2 $CacheDir\fivereborn\citizen\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\*.ttf $CacheDir\fivereborn\citizen\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\ros $CacheDir\fivereborn\citizen\
        Copy-Item -Force -Recurse $WorkDir\data\client\citizen\resources $CacheDir\fivereborn\citizen\
        Copy-Item -Force -Recurse $WorkDir\data\client_rdr\* $CacheDir\fivereborn\
        
        Copy-Item -Force -Recurse C:\f\grpc-ipfs.dll $CacheDir\fivereborn\
    }

    if (!$IsLauncher -and !$IsRDR) {
        Copy-Item -Force $BinRoot\five\release\*.dll $CacheDir\fivereborn\
        Copy-Item -Force $BinRoot\five\release\*.com $CacheDir\fivereborn\

        Copy-Item -Force $BinRoot\five\release\FiveM_Diag.exe $CacheDir\fivereborn\
        Copy-Item -Force -Recurse $BinRoot\five\release\citizen\* $CacheDir\fivereborn\citizen\
    } elseif ($IsLauncher) {
        Copy-Item -Force $BinRoot\launcher\release\*.dll $CacheDir\fivereborn\
        Copy-Item -Force $BinRoot\launcher\release\*.com $CacheDir\fivereborn\
    } elseif ($IsRDR) {
        Copy-Item -Force $BinRoot\rdr3\release\*.dll $CacheDir\fivereborn\
        Copy-Item -Force $BinRoot\rdr3\release\*.com $CacheDir\fivereborn\

        Copy-Item -Force -Recurse $BinRoot\rdr3\release\citizen\* $CacheDir\fivereborn\citizen\
    }
    
    "$GameVersion" | Out-File -Encoding ascii $CacheDir\fivereborn\citizen\version.txt

    if (!$IsLauncher) {
        if (Test-Path $CacheDir\fivereborn\adhesive.dll) {
            Remove-Item -Force $CacheDir\fivereborn\adhesive.dll
        }

        # build compliance stuff
        if (($env:COMPUTERNAME -eq "AVALON") -or ($env:COMPUTERNAME -eq "OMNITRON")) {
            Copy-Item -Force $WorkDir\..\fivem-private\components\adhesive\adhesive.vmp.dll $CacheDir\fivereborn\adhesive.dll

            Push-Location C:\f\bci\
            .\BuildComplianceInfo.exe $CacheDir\fivereborn\ C:\f\bci-list.txt
            Pop-Location
        }
    }

    # build meta/xz variants
    "<Caches>
        <Cache ID=`"fivereborn`" Version=`"$GameVersion`" />
    </Caches>" | Out-File -Encoding ascii $CacheDir\caches.xml

    Copy-Item -Force "$WorkRootDir\tools\ci\xz.exe" xz.exe

    Invoke-Expression "& $WorkRootDir\tools\ci\BuildCacheMeta.exe"

    # build bootstrap executable
    if (!$IsLauncher -and !$IsRDR) {
        Copy-Item -Force $BinRoot\five\release\FiveM.exe CitizenFX.exe
    } elseif ($IsLauncher) {
        Copy-Item -Force $BinRoot\launcher\release\CfxLauncher.exe CitizenFX.exe
    } elseif ($IsRDR) {
        Copy-Item -Force $BinRoot\rdr3\release\CitiLaunch.exe CitizenFX.exe
    }

    if (Test-Path CitizenFX.exe.xz) {
        Remove-Item CitizenFX.exe.xz
    }

    Invoke-Expression "& $WorkRootDir\tools\ci\xz.exe -9 CitizenFX.exe"

    Invoke-WebRequest -Method POST -UseBasicParsing "https://crashes.fivem.net/management/add-version/1.3.0.$GameVersion"

    $uri = 'https://sentry.fivem.net/api/0/organizations/citizenfx/releases/'
    $json = @{
    	version = "1.3.0.$GameVersion"
    	refs = @(
    		@{
    			repository = 'citizenfx/fivem'
    			commit = $env:CI_COMMIT_SHA
    		}
    	)
    	projects = @("fivem-client-1365")
    } | ConvertTo-Json

    $headers = New-Object "System.Collections.Generic.Dictionary[[String],[String]]"
    $headers.Add('Authorization', "Bearer $env:SENTRY_TOKEN")

    Invoke-RestMethod -Uri $uri -Method Post -Headers $headers -Body $json -ContentType 'application/json'

    $LauncherLength = (Get-ItemProperty CitizenFX.exe.xz).Length
    "$LauncherVersion $LauncherLength" | Out-File -Encoding ascii version.txt

    #if (!(Test-Path $WorkDir\caches)) {
    #    New-Item -ItemType SymbolicLink -Force -Path $WorkDir -Name caches -Value $CacheDir
    #}
    Remove-Item -Recurse -Force $WorkDir\caches
    Copy-Item -Recurse -Force $CacheDir $WorkDir\caches
}

if (!$DontUpload) {
    $UploadBranch = $env:CI_ENVIRONMENT_NAME

    Set-Location $CacheDir

    $Branch = $UploadBranch

    $env:Path = "C:\msys64\usr\bin;$env:Path"

    New-Item -ItemType Directory -Force $WorkDir\upload\$Branch\bootstrap | Out-Null
    New-Item -ItemType Directory -Force $WorkDir\upload\$Branch\content | Out-Null

    Copy-Item -Force CitizenFX.exe.xz $WorkDir\upload\$Branch\bootstrap
    Copy-Item -Force version.txt $WorkDir\upload\$Branch\bootstrap
    Copy-Item -Force caches.xml $WorkDir\upload\$Branch\content
    Copy-Item -Recurse -Force diff\fivereborn\ $WorkDir\upload\$Branch\content\

    $BaseRoot = (Split-Path -Leaf $WorkDir)
    Set-Location (Split-Path -Parent $WorkDir)

    if ($IsLauncher) {
        rsync -r -a -v -e "$env:RSH_COMMAND" $BaseRoot/upload/ $env:SSH_TARGET_LAUNCHER

        Invoke-WebHook "Built and uploaded a new CfxGL version ($GameVersion) to $UploadBranch! Go and test it!"
    } elseif ($IsRDR) {
        rsync -r -a -v -e "$env:RSH_COMMAND" $BaseRoot/upload/ $env:SSH_TARGET_RDR

        Invoke-WebHook "Built and uploaded a new RedM version ($GameVersion) to $UploadBranch! Go and test it!"
    } else {
        rsync -r -a -v -e "$env:RSH_COMMAND" $BaseRoot/upload/ $env:SSH_TARGET

        Invoke-WebHook "Built and uploaded a new $env:CI_PROJECT_NAME version ($GameVersion) to $UploadBranch! Go and test it!"
    }

	[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

    # clear cloudflare cache
    $headers = New-Object "System.Collections.Generic.Dictionary[[String],[String]]"
    $headers.Add("X-Auth-Email", $env:CLOUDFLARE_EMAIL)
    $headers.Add("X-Auth-Key", $env:CLOUDFLARE_KEY)

    $uri = 'https://api.cloudflare.com/client/v4/zones/783470409082113ad973c9bb845b62e5/purge_cache'
    $json = @{
        purge_everything=$true
    } | ConvertTo-Json

    #Invoke-RestMethod -Uri $uri -Method Delete -Headers $headers -Body $json -ContentType 'application/json'
}
