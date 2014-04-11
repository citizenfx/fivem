	solution "CitizenMP"
		configurations { "Debug NY", "Release NY" }
		
		flags { "StaticRuntime", "No64BitChecks", "Symbols" }
		
		includedirs { "shared/", "client/shared/", "../vendor/jitasm/" }
		
		configuration "Debug*"
			targetdir "bin/debug"
			defines "_DEBUG"
			
		configuration "Release*"
			targetdir "bin/release"
			defines "NDEBUG"
			optimize "Speed"
			
		configuration "* NY"
			defines "GTA_NY"
			
	project "CitiLaunch"
		targetname "CitizenMP"
		language "C++"
		kind "WindowedApp"
		
		links { "Shared" }
		
		files
		{
			"client/launcher/**.cpp", "client/launcher/**.h", 
			"client/launcher/launcher.rc"
		}
		
	project "Shared"
		targetname "shared"
		language "C++"
		kind "StaticLib"
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}