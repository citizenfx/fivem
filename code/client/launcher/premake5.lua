-- This env var is originating from and must be kept in sync with cfx-build-toolkit/setupEnv.ps1
local gameDumpsRoot = os.getenv("CFX_BUILD_TOOLKIT_GAME_DUMPS_ROOT")

if not gameDumpsRoot or not os.isdir(gameDumpsRoot) then
	gameDumpsRoot = "C:\\f"
end

local gameBuilds = require("../../premake5_builds")

local delayDLLs = {
}

if os.istarget('windows') then
	delayDLLs = {
		"d3d11.dll",
		"d2d1.dll",
		"d3dcompiler_47.dll",
		"dwrite.dll",
		"ole32.dll",
		"shcore.dll",
		"api-ms-win-core-winrt-error-l1-1-1.dll",
		"api-ms-win-core-winrt-l1-1-0.dll",
		"api-ms-win-core-winrt-error-l1-1-0.dll",
		"api-ms-win-core-winrt-string-l1-1-0.dll",
		"api-ms-win-shcore-stream-winrt-l1-1-0.dll",
		"version.dll",
		"dwmapi.dll",
		"bcrypt.dll",
		"wininet.dll",
		"rpcrt4.dll",
		"winmm.dll",
		"dbghelp.dll",
		"shlwapi.dll",
		"ws2_32.dll",
	}
end

do
	local delayList = ""

	for _, v in ipairs(delayDLLs) do
		delayList = delayList .. ("L\"%s\",\n"):format(v)
	end

	if io.readfile('DelayList.h') ~= delayList then
		io.writefile('DelayList.h', delayList)
	end
end

-- is updater?
local function isLauncherPersonality(name)
	return name == 'main'
end

-- is game host?
local function isGamePersonality(name, strict)
	if strict == nil then
		strict = false
	end

	if _OPTIONS['game'] ~= 'five' and _OPTIONS['game'] ~= 'rdr3' and _OPTIONS['game'] ~= 'ny' and strict == false then
		return isLauncherPersonality(name)
	end

	if name == 'game_mtl' then
		return true
	end
	
	if isLauncherPersonality(name) and strict == false then
		filter { "configurations:Debug" }
		return true
	end
	
	return gameBuilds[_OPTIONS['game']][name] ~= nil
end

local function getGameBuild(name)
	if name == 'game_mtl' then
        return 'mtl'
    end
	return gameBuilds[_OPTIONS['game']][name]
end

local function getGameDump(name, gameBuild)
    local gameDump

    if _OPTIONS['game'] == 'five' then
        gameDump = ("%s\\GTA5_%s_dump.exe"):format(gameDumpsRoot, gameBuild)
    elseif _OPTIONS['game'] == 'rdr3' then
        gameDump = ("%s\\RDR2_%s_dump.exe"):format(gameDumpsRoot, gameBuild)
    end

    if name == 'game_mtl' then
        gameDump = ("%s\\Launcher.exe"):format(gameDumpsRoot)
    end

    return gameDump
end

local function launcherpersonality_inner(name)
	local projectName = name == 'main' and 'CitiLaunch' or ('CitiLaunch_' .. name)
	local subprocessName = name

	-- suffix '_aslr' for game processes so these match older binaries
	if name:sub(1, 5) == 'game_' and _OPTIONS['game'] ~= 'ny' and name ~= 'game_mtl' then
		projectName = projectName .. '_aslr'
		subprocessName = subprocessName .. '_aslr'
	end

	if not isLauncherPersonality(name) then
		group 'subprocess'
	end

	project(projectName)
		language "C++"
		kind "WindowedApp"
		
		defines { "COMPILING_LAUNCH", "COMPILING_LAUNCHER" }
		
		defines("LAUNCHER_PERSONALITY_" .. name:upper())

		if name:match('^game_') then
			defines "LAUNCHER_PERSONALITY_ANY_GAME"
		end

		flags { "NoManifest", "NoImportLib" }
		
		symbols "Full"
		buildoptions "/MP"
		
		links { "dbghelp", "psapi", "comctl32", "wininet", "winhttp", "crypt32" }
		
		if isLauncherPersonality(name) then
			add_dependencies { 'vendor:minhook-crt' }
		else
			targetextension '.bin'
		end
		
		dependson { "retarget_pe", "pe_debug" }

		files
		{
			"**.cpp", "**.h", 
			"launcher.rc", "launcher.def",
			"../common/Error.cpp",
			"../common/CfxLocale.Win32.cpp",
		}

		-- remove MAIN-only files so that these don't trigger a rebuild in child procs
		if not isLauncherPersonality(name) then
			removefiles {
				"Bootstrap.cpp",
				"CrossBuildLaunch.cpp",
				"DisableNVSP.cpp",
				"Download.cpp",
				"GameSelect.cpp",
				"Installer.cpp",
				"Updater.cpp",
				"UpdaterUI.cpp",
				"MiniDump.cpp",
				"Minidump.Symbolication.cpp",
			}
		end
		
		if isGamePersonality(name) then
			local gameBuild = getGameBuild(name)
			local gameDump = getGameDump(name, gameBuild)

			if gameDump then
				postbuildcommands {
					("if exist %s ( %%{cfg.buildtarget.directory}\\retarget_pe \"%%{cfg.buildtarget.abspath}\" %s )"):format(
						gameDump, gameDump
					)
				}
			end

			if gameBuild then
				postbuildcommands {
					("if exist \"%s\" ( %%{cfg.buildtarget.directory}\\pe_debug \"%%{cfg.buildtarget.abspath}\" \"%s\" )"):format(
						path.getabsolute(('../../tools/dbg/dump_%s.txt'):format(gameBuild)),
						path.getabsolute(('../../tools/dbg/dump_%s.txt'):format(gameBuild))
					)
				}
			end

			resign()
		end
		
		filter {}
		
		if isLauncherPersonality(name) then
			postbuildcommands {
				"echo. > %{cfg.buildtarget.abspath}.formaldev",
			}
		end
		
		pchsource "StdInc.cpp"
		pchheader "StdInc.h"

		add_dependencies { 'vendor:tinyxml2', 'vendor:concurrentqueue' }

		if isLauncherPersonality(name) then
			staticruntime 'On'
			links "SharedLibc"
			add_dependencies { 'vendor:xz-crt', 'vendor:zstd-crt', 'vendor:minizip-crt', 'vendor:tbb-crt', 'vendor:boost_locale-crt', 'vendor:breakpad-crt' }
			add_dependencies { 'vendor:curl-crt', 'vendor:cpr-crt', 'vendor:mbedtls_crt', 'vendor:hdiffpatch-crt', 'vendor:openssl_crypto_crt' }
		else
			links "Shared"
			add_dependencies { 'vendor:tbb', 'vendor:boost_locale', 'vendor:breakpad', 'vendor:openssl_crypto' }
		end

		filter { "options:game=ny" }
			targetname "LibertyM"

		filter { "options:game=payne" }
			targetname "CitizenPayne"

		filter { "options:game=five" }
			targetname "VMP"
			
		filter { "options:game=launcher" }
			targetname "CfxLauncher"

		filter {}
			
		if name ~= 'main' then
			targetname("CitizenFX_SubProcess_" .. subprocessName)
		end
		
		linkoptions "/IGNORE:4254 /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get
		
		if isGamePersonality(name, true) then
			defines("LAUNCHER_PERSONALITY_GAME")
		end

		if isGamePersonality(name) then
			-- VS14 linker behavior change causes the usual workaround to no longer work, use an undocumented linker switch instead
			-- note that pragma linker directives ignore these (among other juicy arguments like /WBRDDLL, /WBRDTESTENCRYPT and other
			-- Microsoft Warbird-specific DRM functions... third-party vendors have to handle their own way of integrating
			-- PE parsing and writing, yet Microsoft has their feature hidden in the exact linker those vendors use...)
			linkoptions "/LAST:.zdata"

			-- V8 requires a 1.5 MB stack at minimum (default is 1 MB stack space for V8 only, so 512 kB safety)
			linkoptions "/STACK:0x180000"

			-- for debug builds, we will load at the default base to allow easier copy/paste of addresses from disassembly
			filter { "configurations:Debug" }
				linkoptions { "/SAFESEH:NO", "/DYNAMICBASE:NO" }

			-- add NOTHING below here (`filter` from `isGamePersonality` would break, otherwise)
		end
		
		-- reset isGamePersonality bit
		filter {}

		if name == 'chrome' then
			-- Chrome is built with an 8MB initial stack, and render processes' V8 stack limit assumes such as well
			linkoptions "/STACK:0x800000"
		end
			

		for _, dll in ipairs(delayDLLs) do
			linkoptions("/DELAYLOAD:" .. dll)
		end

	group ''
end

local function launcherpersonality(name)
	launcherpersonality_inner(name)
end

launcherpersonality 'main'
launcherpersonality 'chrome'

local builds = gameBuilds[_OPTIONS["game"]]
if builds ~= nil then
	for key, _ in pairs(builds) do
		launcherpersonality(key)
	end
end

if _OPTIONS['game'] == 'five' then
	launcherpersonality 'game_mtl'
elseif _OPTIONS['game'] == 'rdr3' then
	launcherpersonality 'game_mtl'
end

group 'subprocess'
project 'CitiLaunch_TLSDummy'
	kind 'SharedLib'
	language 'C++'

	defines { 'IS_TLS_DLL' }

	staticruntime 'On'

	files {
		'DummyVariables.cpp'
	}

group ''
