	solution "CitizenMP"
		configurations { "Debug NY", "Release NY" }
		
		flags { "StaticRuntime", "No64BitChecks", "Symbols", "Unicode" }
		
		flags { "NoIncrementalLink", "NoEditAndContinue" } -- this breaks our custom section ordering in citilaunch, and is kind of annoying otherwise
		
		includedirs { "shared/", "client/shared/", "../vendor/jitasm/", os.getenv("BOOST_ROOT") }
	
		configuration "Debug*"
			targetdir "bin/debug"
			defines "NDEBUG"
			
		configuration "Release*"
			targetdir "bin/release"
			defines "NDEBUG"
			optimize "Speed"
			
		configuration "* NY"
			defines "GTA_NY"
			
	project "CitiLaunch"
		targetname "CitizenFX"
		language "C++"
		kind "WindowedApp"
		
		links { "Shared" }
		
		files
		{
			"client/launcher/**.cpp", "client/launcher/**.h", 
			"client/launcher/launcher.rc", "client/launcher/launcher.def"
		}
		
		configuration "windows"
			linkoptions "/ENTRY:main /IGNORE:4254 /DYNAMICBASE:NO /SAFESEH:NO" -- 4254 is the section type warning we tend to get
		
	project "CitiGame"
		targetname "CitizenGame"
		language "C++"
		kind "SharedLib"
		
		files
		{
			"client/citigame/**.cpp", "client/citigame/**.h"
		}
		
		links { "Shared", "yaml-cpp", "lua51", "winmm", "winhttp", "ws2_32" }
		
		defines "COMPILING_GAME"
		
		libdirs { "../vendor/luajit/src/" }
		includedirs { "client/citigame/include/", "../vendor/luajit/src/", "../vendor/yaml-cpp/include/" }
		
		configuration "* NY"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/citigame/net/" }
			links { "HooksNY", "GameNY" }
			
	project "GameNY"
		targetname "game_ny"
		language "C++"
		kind "SharedLib"
		
		links { "Shared" }
		
		defines "COMPILING_GAMESPEC"
		
		configuration "* NY"
			includedirs { "client/game_ny/base/" }
		
			files
			{
				"client/game_ny/**.cpp", "client/game_ny/**.h"
			}
			
	project "HooksNY"
		targetname "hooks_ny"
		language "C++"
		kind "SharedLib"
		
		links { "Shared", "GameNY", "ws2_32" }
		
		defines "COMPILING_HOOKS"		
		
		configuration "* NY"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/hooks_ny/base/" }
			files
			{
				"client/hooks_ny/**.cpp", "client/hooks_ny/**.h"
			}
		
	project "Shared"
		targetname "shared"
		language "C++"
		kind "StaticLib"
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}
		
	project "yaml-cpp"
		targetname "yaml-cpp"
		language "C++"
		kind "StaticLib"
		
		includedirs { "../vendor/yaml-cpp/include" }
		
		files
		{
			"../vendor/yaml-cpp/src/*.cpp"
		}