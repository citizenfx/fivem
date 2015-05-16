	project "CitiLaunch"
		language "C++"
		kind "WindowedApp"
		
		defines "COMPILING_LAUNCH"
		
		links { "SharedLibc", "dbghelp", "psapi", "comctl32", "breakpad", "wininet", "winhttp" }

		files
		{
			"**.cpp", "**.h", 
			"launcher.rc", "launcher.def",
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

		configuration "game=five"
			targetname "FiveM"
		
		configuration "windows"
			linkoptions "/ENTRY:main /IGNORE:4254 /DYNAMICBASE:NO /SAFESEH:NO /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get

		filter "architecture:x64"
			links { "libcurlx64" }

		filter { "architecture:x64", "Debug" }
			links { "liblzmax64d" }

		filter { "architecture:x64", "Release" }
			links { "liblzmax64" }

		filter "architecture:not x64"
			links { "libcurl", "liblzma" }