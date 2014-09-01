	solution "CitizenMP"
		configurations { "Debug NY", "Release NY" }
		
		flags { "StaticRuntime", "No64BitChecks", "Symbols", "Unicode" }
		
		flags { "NoIncrementalLink", "NoEditAndContinue" } -- this breaks our custom section ordering in citilaunch, and is kind of annoying otherwise
		
		includedirs { "shared/", "client/shared/", "../vendor/jitasm/", "deplibs/include/", "../vendor/gtest/include/", os.getenv("BOOST_ROOT") }
		
		defines { "GTEST_HAS_PTHREAD=0" }
		
		libdirs { "deplibs/lib/" }
		
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
		
		defines "COMPILING_LAUNCH"
		
		links { "Shared", "dbghelp", "psapi", "libcurl", "tinyxml", "liblzma", "comctl32" }
		
		files
		{
			"client/launcher/**.cpp", "client/launcher/**.h", 
			"client/launcher/launcher.rc", "client/launcher/launcher.def",
			"client/common/Error.cpp"
		}
		
		pchsource "client/launcher/StdInc.cpp"
		pchheader "StdInc.h"
		
		libdirs { "client/libcef/lib/" }
		
		linkoptions "/DELAYLOAD:libcef.dll"
		
		includedirs { "client/libcef/" }
		
		configuration "Debug*"
			links { "cef_sandboxd", "libcefd" }
			
		configuration "Release*"
			links { "cef_sandbox", "libcef" }
		
		configuration "windows"
			linkoptions "/ENTRY:main /IGNORE:4254 /DYNAMICBASE:NO /SAFESEH:NO /LARGEADDRESSAWARE" -- 4254 is the section type warning we tend to get
		
	project "CitiGame"
		targetname "CitizenGame"
		language "C++"
		kind "SharedLib"
		
		files
		{
			"client/citigame/**.cpp", "client/citigame/**.h", "client/common/Error.cpp", "client/citigame/**.c", "client/common/StdInc.cpp"
		}
		
		links { "Shared", "yaml-cpp", "msgpack-c", "lua51", "winmm", "winhttp", "ws2_32", "libcef_dll", "libcef", "delayimp", "libnp" }
		
		defines "COMPILING_GAME"
		
		libdirs { "../vendor/luajit/src/", "client/libcef/lib/", "client/shared/np" }
		includedirs { "client/citigame/include/", "../vendor/luajit/src/", "../vendor/yaml-cpp/include/", "../vendor/msgpack-c/src/", "deplibs/include/msgpack-c/", "client/libcef/", "client/shared/np" }
		
		linkoptions "/DELAYLOAD:libcef.dll"
		
		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
		
		configuration "* NY"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/citigame/net/" }
			links { "HooksNY", "GameNY" }
			
		configuration "Debug*"
			links { "libcefd" }
			
		configuration "Release*"
			links { "libcef" }
			
	project "GameNY"
		targetname "game_ny"
		language "C++"
		kind "SharedLib"
		
		links { "Shared", "zlib" }
		
		defines "COMPILING_GAMESPEC"
		
		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
		
		configuration "* NY"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "../vendor/zlib/" }
		
			files
			{
				"client/game_ny/**.cpp", "client/game_ny/**.h", "client/common/Error.cpp", "client/common/StdInc.cpp"
			}
			
	project "HooksNY"
		targetname "hooks_ny"
		language "C++"
		kind "SharedLib"
		
		links { "Shared", "GameNY", "ws2_32" }
		
		defines "COMPILING_HOOKS"	

		pchsource "client/hooks_ny/base/StdInc.cpp"
		pchheader "StdInc.h"		
		
		configuration "* NY"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/hooks_ny/base/" }
			
			files
			{
				"client/hooks_ny/**.cpp", "client/hooks_ny/**.h", "client/common/Error.cpp"
			}
		
	project "Shared"
		targetname "shared"
		language "C++"
		kind "StaticLib"
		
		includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/" }
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}
		
	project "libcef_dll"
		targetname "libcef_dll_wrapper"
		language "C++"
		kind "StaticLib"
		
		defines { "USING_CEF_SHARED", "NOMINMAX", "WIN32" }
		
		flags { "NoIncrementalLink", "NoMinimalRebuild" }
		
		includedirs { ".", "client/libcef" }
		
		buildoptions "/MP"
		
		files
		{
			"client/libcef/libcef_dll/**.cc", "client/libcef/libcef_dll/**.cpp", "client/libcef/libcef_dll/**.h"
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

	project "msgpack-c"
		targetname "msgpack-c"
		language "C++"
		kind "StaticLib"

		includedirs { "../vendor/msgpack-c/src", "../vendor/msgpack-c/src/msgpack", "deplibs/include/msgpack-c/" }

		files
		{
			"../vendor/msgpack-c/src/object.cpp", "../vendor/msgpack-c/src/*.c" 
		}

	project "zlib"
		targetname "zlib"
		language "C"
		kind "StaticLib"

		files { "../vendor/zlib/*.c", "../vendor/zlib/*.h" }
		excludes { "../vendor/zlib/example.c", "../vendor/zlib/minigzip.c" }

		defines { "WIN32" }
		
	project "gtest_main"
		language "C++"
		kind "StaticLib"
		
		includedirs { "../vendor/gtest/" }
		files { "../vendor/gtest/src/gtest-all.cc", "../vendor/gtest/src/gtest_main.cc" }
		
	project "tests_citigame"
		language "C++"
		kind "ConsoleApp"
		
		links { "gtest_main", "CitiGame", "Shared" }
		
		includedirs { "client/citigame/include/" }
		
		files { "tests/citigame/*.cpp" }
