using module .\cfxBuildContext.psm1

class CfxBuildTools {
    [string] $SevenZip
    [string] $xz
    [string] $tar
    [string] $premake
    [string] $nuget
    [string] $vswhere

    [string] $gci

    CfxBuildTools([CfxBuildContext] $ctx) {
        $this.ctx = $ctx
    }
    
    [CfxBuildContext] hidden $ctx

    [string] hidden $_dumpsyms
    [boolean] hidden $_dumpsymsVerified = $false
    [string] getDumpSyms() {
        if (!$this._dumpsymsVerified) {
            $dumpsymsPath = $this.ctx.getPathInBuildCache("dump_syms/dump_syms.exe")
            $msdia140Path = $this.ctx.getPathInBuildCache("dump_syms/msdia140.dll")

            if (!(Test-Path $dumpsymsPath) -or !(Test-Path $msdia140Path)) {
                if (!$this.ctx.VSDir) {
                    throw "No Visual Studio directory, make sure Invoke-SetupVS has run"
                }

                $dumpsymsOriginalPath = $this.ctx.getPathInProject("vendor\breakpad\src\tools\windows\binaries\dump_syms.exe")
                $msdia140OriginalPath = [IO.Path]::Combine($this.ctx.VSDir, "DIA SDK\bin\msdia140.dll")
    
                if (!(Test-Path $msdia140OriginalPath)) {
                    throw "$msdia140OriginalPath not found"
                }
    
                New-Item -ItemType Directory -Force ([IO.Path]::GetDirectoryName($dumpsymsPath))
    
                Copy-Item -ErrorAction Stop -Path $dumpsymsOriginalPath -Destination $dumpsymsPath
                Copy-Item -ErrorAction Stop -Path $msdia140OriginalPath -Destination $msdia140Path
            }

            $this._dumpsyms = $dumpsymsPath
            $this._dumpsymsVerified = $true
        }

        return $this._dumpsyms
    }

    [string] hidden $_MSYS2Root
    [boolean] hidden $_MSYS2RootVerified = $false
    [string] getMSYS2Root() {
        if (!$this._MSYS2RootVerified) {
            $msys2Root = "C:\msys64"

            if ($env:MSYS2_ROOT) {
                $msys2Root = $env:MSYS2_ROOT
            }

            if (!(Test-Path $msys2Root)) {
                throw "MSYS2 directory at $msys2Root does not exist"
            }

            $this._MSYS2Root = $msys2Root.TrimEnd("\")
            $this._MSYS2RootVerified = $true
        }

        return $this._MSYS2Root
    }

    [string] hidden $_pacman
    [boolean] hidden $_pacmanVerified = $false
    [string] getPacman() {
        if (!$this._pacmanVerified) {
            $msys2Root = $this.getMSYS2Root()

            $pacmanPath = "$msys2Root\usr\bin\pacman.exe"

            if (!(Test-Path $pacmanPath)) {
                throw "Pacman not found, make sure MSYS2 is installed"
            }

            $this._pacman = $pacmanPath
            $this._pacmanVerified = $true
        }

        return $this._pacman
    }

    [string] hidden $_rsync
    [boolean] hidden $_rsyncVerified = $false
    [string] getRsync() {
        if (!$this._rsyncVerified) {
            $msys2Root = $this.getMSYS2Root()

            $rsyncPath = "$msys2Root\usr\bin\rsync.exe"

            if (!(Test-Path $rsyncPath)) {
                $pacman = $this.getPacman()

                Write-Host "Rsync is not installed, installing now"
    
                & $pacman --noconfirm --needed -Sy rsync
                Test-LastExitCode "Failed to install rsync"

                Write-Host "Rsync has been installed"
            }

            $this._rsync = $rsyncPath
            $this._rsyncVerified = $true
        }

        return $this._rsync
    }

    # Ensure we have a symstore.exe path from Windows Kit 10 Debuggers
    [string] hidden $_symstore
    [boolean] hidden $_symstoreVerified = $false
    [string] getSymstore() {
        if (!$this._symstoreVerified) {
            $kitInstallDir = ""

            $symstoreFromKitPath = "Debuggers\x64\symstore.exe"
    
            $naiveKitInstallDir = "C:\Program Files (x86)\Windows Kits\10"
            $kitRegistryKey = "HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots"
    
            if (Test-Path $naiveKitInstallDir) {
                $kitInstallDir = $naiveKitInstallDir
            }
            elseif (Test-Path $kitRegistryKey) {
                $kitInstallDir = (Get-ItemProperty $kitRegistryKey).KitsRoot10
            }

            $symstorePath = [IO.Path]::Combine($kitInstallDir, $symstoreFromKitPath)
    
            if (!(Test-Path $symstorePath)) {
                throw "symstore.exe not found at $symstorePath"
            }

            $this._symstore = $symstorePath
            $this._symstoreVerified = $true
        }

        return $this._symstore
    }

    [string] hidden $_bcm
    [boolean] hidden $_bcmVerified = $false
    [string] getBCM() {
        if (!$this._bcmVerified) {
            $bcmDir = $this.ctx.getPathInBuildCache("build-cache-meta")
            $bcmPath = "$bcmDir\buildcachemeta-go.exe"
            $bcmURL = "https://github.com/citizenfx/buildcachemeta-go/releases/download/v0.0.4/buildcachemeta-go_0.0.4_win32_amd64.tar.gz"

            if (!(Test-Path $bcmPath)) {
                New-Item -ItemType Directory -Force $bcmDir

                curl.exe -Lo $bcmDir\bcm.tar.gz $bcmURL
                Test-LastExitCode "Failed to fetch buildcachemeta tool"

                & $this.tar -C $bcmDir -xvf $bcmDir\bcm.tar.gz
                Test-LastExitCode "Failed to unpack buildcachemeta tool"

                Remove-Item -Force $bcmDir\bcm.tar.gz

                if (!(Test-Path $bcmPath)) {
                    throw "Failed to verify buildcachemeta tool"
                }
            }
            
            $this._bcm = $bcmPath
            $this._bcmVerified = $true
        }

        return $this._bcm
    }

    [string] hidden $_sentryCLI
    [boolean] hidden $_sentryCLIVerified = $false
    [string] getSentryCLI() {
        if (!$this._sentryCLIVerified) {
            $sentryDir = $this.ctx.getPathInBuildCache("sentry")
            $sentryCLIPath = "$sentryDir\sentry-cli-1.67.2.exe"
            $sentryCLIURL = "https://content.cfx.re/mirrors/vendor/sentry/sentry-cli-1.67.2.exe"

            if (!(Test-Path $sentryCLIPath)) {
                New-Item -ItemType Directory -Force $sentryDir

                curl.exe -Lo $sentryCLIPath $sentryCLIURL
                Test-LastExitCode "Failed to fetch sentry-cli"
            }

            $this._sentryCLI = $sentryCLIPath
            $this._sentryCLIVerified = $true
        }

        return $this._sentryCLI
    }

    [void] ensurePython() {
        if ($null -eq (Get-Command -ErrorAction Ignore python)) {
            throw "Python not found, make sure it is installed and available in the PATH env var"
        }

        # we need py launcher too
        if ($null -eq (Get-Command -ErrorAction Ignore py)) {
            throw "The required 'py' command is not available in the PATH env var"
        }
    }

    [void] ensureNodeJS() {
        if ($null -eq (Get-Command -ErrorAction Ignore node)) {
            throw "Node.JS not found, make sure it is installed and available in the PATH env var"
        }
    }

    [void] ensureYarn() {
        if ($null -eq (Get-Command -ErrorAction Ignore yarn)) {
            throw "Yarn not found, make sure it is installed and available in the PATH env var"
        }
    }
}

function Get-CfxBuildTools {
    param(
        [CfxBuildContext] $Context
    )

    $tools = [CfxBuildTools]::new($Context)

    $tools.tar = "$env:WINDIR\system32\tar.exe"

    $tools.SevenZip = $Context.getPathInProject("code\tools\ci\7z.exe")
    $tools.xz = $Context.getPathInProject("code\tools\ci\xz.exe")
    $tools.vswhere = $Context.getPathInProject("code\tools\ci\vswhere.exe")
    $tools.premake = $Context.getPathInProject("code\tools\ci\premake5.exe")
    $tools.nuget = $Context.getPathInProject("code\tools\ci\nuget.exe")

    $tools.gci = "C:\f\gci\gci.exe" | ConvertTo-VerifiedPath

    return $tools
}

function ConvertTo-VerifiedPath {
    param(
        [Parameter(ValueFromPipeline)][string] $Path
    )

    if (Test-Path $Path) {
        return $Path
    }

    return ""
}

function Test-PathExists {
    param(
        [string] $Path,
        [string] $ErrorMessage
    )

    if (!(Test-Path $Path)) {
        throw $ErrorMessage
    }
}

function Test-LastExitCode {
    param(
        [string] $ErrorMessage
    )

    if ($LASTEXITCODE -ne 0) {
        throw $ErrorMessage
    }
}

function Invoke-EnsureDirExists {
    param([string] $Path)

    if (!(Test-Path $Path)) {
        New-Item -ItemType Directory -Force $Path
    }

    return $Path
}

function Invoke-RestoreFromBuildCache {
    param(
        [CfxBuildContext] $Context,
        [string] $CacheName,
        [string] $Path
    )
    
    $cachePath = $Context.getPathInBuildCache($CacheName)

    if (!(Test-Path $cachePath)) {
        return
    }

    Move-Item -Force -Path $cachePath -Destination $Path
}

function Invoke-SaveInBuildCache {
    param(
        [CfxBuildContext] $Context,
        [string] $CacheName,
        [string] $Path
    )
    
    $cachePath = $Context.getPathInBuildCache($CacheName)

    if (Test-Path $cachePath) {
        Remove-Item -Force -Recurse $cachePath
    }

    Move-Item -Force -Path $Path -Destination $cachePath
}
