	project "CitiLaunch"
		language "C++"
		kind "WindowedApp"
		
		defines "COMPILING_LAUNCH"

		flags "NoManifest"
		
		symbols "Full"
		
		links { "SharedLibc", "dbghelp", "psapi", "comctl32", "breakpad", "wininet", "winhttp", "Win2D" }
		
		dependson { "retarget_pe", "pe_debug" }

		files
		{
			"**.cpp", "**.h", 
			"launcher.rc", "launcher.def",
			"../common/Error.cpp"
		}
		
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
		
		pchsource "StdInc.cpp"
		pchheader "StdInc.h"

		add_dependencies { 'vendor:breakpad', 'vendor:tinyxml2', 'vendor:xz-crt', 'vendor:curl-crt', 'vendor:cpr-crt', 'vendor:minizip-crt', 'vendor:tbb-crt' }
		
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
		
		configuration "windows"
			linkoptions "/IGNORE:4254 /SAFESEH:NO /DYNAMICBASE:NO /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get
			linkoptions "/DELAYLOAD:d2d1.dll /DELAYLOAD:d3dcompiler_47.dll /DELAYLOAD:dwrite.dll /DELAYLOAD:ole32.dll /DELAYLOAD:shcore.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-1.dll /DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-0.dll /DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll /DELAYLOAD:api-ms-win-shcore-stream-winrt-l1-1-0.dll"

			-- VS14 linker behavior change causes the usual workaround to no longer work, use an undocumented linker switch instead
			-- note that pragma linker directives ignore these (among other juicy arguments like /WBRDDLL, /WBRDTESTENCRYPT and other
			-- Microsoft Warbird-specific DRM functions... third-party vendors have to handle their own way of integrating
			-- PE parsing and writing, yet Microsoft has their feature hidden in the exact linker those vendors use...)
			linkoptions "/LAST:.zdata"

	externalproject "Win2D"
		if os.getenv('COMPUTERNAME') ~= 'AVALON' and not os.getenv('CI') then
			filename "../../../vendor/win2d/winrt/lib/winrt.lib.uap"
		else
			filename "../../vendor/win2d/winrt/lib/winrt.lib.uap"
		end
		uuid "26b85b6e-3520-42b5-adb6-971010cc99fa"
		kind "StaticLib"
		language "C++"
