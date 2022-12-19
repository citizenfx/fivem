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
local function isGamePersonality(name)
	if _OPTIONS['game'] ~= 'five' and _OPTIONS['game'] ~= 'rdr3' and _OPTIONS['game'] ~= 'ny' then
		return isLauncherPersonality(name)
	end

	if name == 'game_mtl' then
		return true
	end

	if name == 'game_1604' or name == 'game_2060' or name == 'game_372' or name == 'game_2189' or name == 'game_2372' or name == 'game_2545' or name == 'game_2612' or name == 'game_2699' or name == 'game_2802' then
		return true
	end
	
	if name == 'game_1311' or name == 'game_1355' or name == 'game_1436' or name == 'game_1491' then
		return true
	end
	
	if name == 'game_43' then
		return true
	end
	
	if isLauncherPersonality(name) then
		filter { "configurations:Debug" }
		return true
	end
	
	return false
end

local function launcherpersonality_inner(name, aslr)
	local projectName = name == 'main' and 'CitiLaunch' or ('CitiLaunch_' .. name)

	if aslr then
		projectName = projectName .. '_aslr'
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
			local gameBuild
			local gameDump

			if _OPTIONS['game'] == 'five' then
				gameBuild = '1604'

				if name == 'game_2802' then gameBuild = '2802_0' end
				if name == 'game_2699' then gameBuild = '2699_0' end
				if name == 'game_2612' then gameBuild = '2612_1' end
				if name == 'game_2545' then gameBuild = '2545_0' end
				if name == 'game_2372' then gameBuild = '2372_0' end
				if name == 'game_2189' then gameBuild = '2189_0' end
				if name == 'game_2060' then gameBuild = '2060_2' end
				if name == 'game_372' then gameBuild = '372' end

				gameDump = ("C:\\f\\GTA5_%s_dump.exe"):format(gameBuild)
			elseif _OPTIONS['game'] == 'rdr3' then
				gameBuild = '1311'

				if name == 'game_1355' then gameBuild = '1355_18' end
				if name == 'game_1436' then gameBuild = '1436_31' end
				if name == 'game_1491' then gameBuild = '1491_16' end

				gameDump = ("C:\\f\\RDR2_%s.exe"):format(gameBuild)
			end

			if name == 'game_mtl' then
				gameDump = "C:\\f\\Launcher.exe"
				gameBuild = 'mtl'
			end

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
			targetname "FiveM"
			
		filter { "options:game=launcher" }
			targetname "CfxLauncher"

		filter {}
			
		if name ~= 'main' then
			targetname("CitizenFX_SubProcess_" .. name .. (aslr and "_aslr" or ""))
		end
		
		linkoptions "/IGNORE:4254 /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get
		
		if isGamePersonality(name) then
			-- VS14 linker behavior change causes the usual workaround to no longer work, use an undocumented linker switch instead
			-- note that pragma linker directives ignore these (among other juicy arguments like /WBRDDLL, /WBRDTESTENCRYPT and other
			-- Microsoft Warbird-specific DRM functions... third-party vendors have to handle their own way of integrating
			-- PE parsing and writing, yet Microsoft has their feature hidden in the exact linker those vendors use...)
			linkoptions "/LAST:.zdata"

			-- V8 requires a 1.5 MB stack at minimum (default is 1 MB stack space for V8 only, so 512 kB safety)
			linkoptions "/STACK:0x180000"

			if not aslr and not isLauncherPersonality(name) then
				linkoptions { "/SAFESEH:NO", "/DYNAMICBASE:NO" }
			else
				filter { "configurations:Debug" }
					linkoptions { "/SAFESEH:NO", "/DYNAMICBASE:NO" }
			end

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
	launcherpersonality_inner(name, false)

	if name:sub(1, 5) == 'game_' and _OPTIONS['game'] ~= 'ny' and name ~= 'game_mtl' then
		launcherpersonality_inner(name, true)
	end
end

launcherpersonality 'main'
launcherpersonality 'chrome'

if _OPTIONS['game'] == 'five' then
	launcherpersonality 'game_1604'
	--launcherpersonality 'game_372'
	launcherpersonality 'game_2060'
	launcherpersonality 'game_2189'
	launcherpersonality 'game_2372'
	launcherpersonality 'game_2545'
	launcherpersonality 'game_2612'
	launcherpersonality 'game_2699'
	launcherpersonality 'game_2802'
	launcherpersonality 'game_mtl'
elseif _OPTIONS['game'] == 'rdr3' then
	launcherpersonality 'game_1311'
	launcherpersonality 'game_1355'
	launcherpersonality 'game_1436'
	launcherpersonality 'game_1491'
	launcherpersonality 'game_mtl'
elseif _OPTIONS['game'] == 'ny' then
	launcherpersonality 'game_43'
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
