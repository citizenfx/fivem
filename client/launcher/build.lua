	project "CitiLaunch"
		language "C++"
		kind "WindowedApp"
		
		defines "COMPILING_LAUNCH"
		
		links { "SharedLibc", "dbghelp", "psapi", "libcurl", "tinyxml2", "liblzma", "comctl32", "breakpad", "wininet", "winhttp" }
		
		files
		{
			"**.cpp", "**.h", 
			"launcher.rc", "client/launcher/launcher.def",
			"../common/Error.cpp"
		}
		
		pchsource "StdInc.cpp"
		pchheader "StdInc.h"

		add_dependencies { 'vendor:breakpad', 'vendor:tinyxml2' }
		
		--includedirs { "client/libcef/", "../vendor/breakpad/src/", "../vendor/tinyxml2/" }

		flags { "StaticRuntime" }

		configuration "game=ny"
			targetname "CitizenFX"

		configuration "game=payne"
			targetname "CitizenPayne"
		
		configuration "windows"
			linkoptions "/ENTRY:main /IGNORE:4254 /DYNAMICBASE:NO /SAFESEH:NO /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get