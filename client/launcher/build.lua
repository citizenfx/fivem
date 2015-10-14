	project "CitiLaunch"
		language "C++"
		kind "WindowedApp"
		
		defines "COMPILING_LAUNCH"

		flags "NoManifest"
		
		links { "SharedLibc", "dbghelp", "psapi", "comctl32", "breakpad", "wininet", "winhttp" }

		files
		{
			"**.cpp", "**.h", 
			"launcher.rc", "launcher.def",
			"../common/Error.cpp"
		}
		
		pchsource "StdInc.cpp"
		pchheader "StdInc.h"

		add_dependencies { 'vendor:breakpad', 'vendor:tinyxml2', 'vendor:xz-crt', 'vendor:curl-crt' }
		
		--includedirs { "client/libcef/", "../vendor/breakpad/src/", "../vendor/tinyxml2/" }

		flags { "StaticRuntime" }

		configuration "game=ny"
			targetname "CitizenFX"

		configuration "game=payne"
			targetname "CitizenPayne"

		configuration "game=five"
			targetname "FiveM"
		
		configuration "windows"
			linkoptions "/IGNORE:4254 /DYNAMICBASE:NO /SAFESEH:NO /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get

			-- VS14 linker behavior change causes the usual workaround to no longer work, use an undocumented linker switch instead
			-- note that pragma linker directives ignore these (among other juicy arguments like /WBRDDLL, /WBRDTESTENCRYPT and other
			-- Microsoft Warbird-specific DRM functions... third-party vendors have to handle their own way of integrating
			-- PE parsing and writing, yet Microsoft has their feature hidden in the exact linker those vendors use...)
			linkoptions "/LAST:.zdata"
