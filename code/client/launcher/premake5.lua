-- is updater?
local function isLauncherPersonality(name)
	return name == 'main'
end

-- is game host?
local function isGamePersonality(name)
	return name == 'main'
end

local function launcherpersonality(name)
	local projectName = name == 'main' and 'CitiLaunch' or ('CitiLaunch_' .. name)

	project(projectName)
		language "C++"
		kind "WindowedApp"
		
		defines "COMPILING_LAUNCH"
		
		defines("LAUNCHER_PERSONALITY_" .. name:upper())

		flags "NoManifest"
		
		symbols "Full"
		
		links { "SharedLibc", "dbghelp", "psapi", "comctl32", "breakpad", "wininet", "winhttp", "crypt32" }
		
		if isLauncherPersonality(name) then
			links "Win2D"
		else
			targetextension '.bin'
		end
		
		dependson { "retarget_pe", "pe_debug" }

		files
		{
			"**.cpp", "**.h", 
			"launcher.rc", "launcher.def",
			"../common/Error.cpp"
		}
		
		if isGamePersonality(name) then
			if _OPTIONS['game'] == 'five' then
				postbuildcommands {
					("copy /y \"%s\" \"%%{cfg.buildtarget.directory}\""):format(
						path.getabsolute('../../tools/dbg/bin/msobj140.dll'):gsub('/', '\\')
					),
					("copy /y \"%s\" \"%%{cfg.buildtarget.directory}\""):format(
						path.getabsolute('../../tools/dbg/bin/mspdbcore.dll'):gsub('/', '\\')
					),
					"if exist C:\\f\\GTA5_1604_dump.exe ( %{cfg.buildtarget.directory}\\retarget_pe \"%{cfg.buildtarget.abspath}\" C:\\f\\GTA5_1604_dump.exe )",
					("%%{cfg.buildtarget.directory}\\pe_debug \"%%{cfg.buildtarget.abspath}\" \"%s\""):format(
						path.getabsolute('../../tools/dbg/dump_1604.txt')
					)
				}
			elseif _OPTIONS['game'] == 'rdr3' then
				postbuildcommands {
					"if exist C:\\f\\RDR2.exe ( %{cfg.buildtarget.directory}\\retarget_pe \"%{cfg.buildtarget.abspath}\" C:\\f\\RDR2.exe )",
				}
			end
		end
		
		pchsource "StdInc.cpp"
		pchheader "StdInc.h"

		add_dependencies { 'vendor:breakpad', 'vendor:tinyxml2', 'vendor:xz-crt', 'vendor:minizip-crt', 'vendor:tbb-crt' }
		
		if isLauncherPersonality(name) then
			add_dependencies { 'vendor:curl-crt', 'vendor:cpr-crt' }
		end
		
		--includedirs { "client/libcef/", "../vendor/breakpad/src/", "../vendor/tinyxml2/" }

		staticruntime 'On'

		configuration "game=ny"
			targetname "CitizenFX"

		configuration "game=payne"
			targetname "CitizenPayne"

		configuration "game=five"
			targetname "FiveM"
			
		configuration "game=launcher"
			targetname "CfxLauncher"
			
		if name ~= 'main' then
			configuration {}
			targetname("CitizenFX_SubProcess_" .. name)
		end
		
		configuration "windows"
			linkoptions "/IGNORE:4254 /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get
			
			if isGamePersonality(name) then
				linkoptions "/SAFESEH:NO /DYNAMICBASE:NO"

				-- VS14 linker behavior change causes the usual workaround to no longer work, use an undocumented linker switch instead
				-- note that pragma linker directives ignore these (among other juicy arguments like /WBRDDLL, /WBRDTESTENCRYPT and other
				-- Microsoft Warbird-specific DRM functions... third-party vendors have to handle their own way of integrating
				-- PE parsing and writing, yet Microsoft has their feature hidden in the exact linker those vendors use...)
				linkoptions "/LAST:.zdata"
			end
			
			linkoptions "/DELAYLOAD:d3d11.dll /DELAYLOAD:d2d1.dll /DELAYLOAD:d3dcompiler_47.dll /DELAYLOAD:dwrite.dll /DELAYLOAD:ole32.dll /DELAYLOAD:shcore.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-1.dll /DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll /DELAYLOAD:api-ms-win-shcore-stream-winrt-l1-1-0.dll"
end

launcherpersonality 'main'
launcherpersonality 'chrome'

externalproject "Win2D"
	if os.getenv('COMPUTERNAME') ~= 'AVALON' and not os.getenv('CI') then
		filename "../../../vendor/win2d/winrt/lib/winrt.lib.uap"
	else
		filename "../../vendor/win2d/winrt/lib/winrt.lib.uap"
	end
	uuid "26b85b6e-3520-42b5-adb6-971010cc99fa"
	kind "StaticLib"
	language "C++"
	