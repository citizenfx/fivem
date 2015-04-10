xpcall(function()
dofile('tools/build/init.lua')

-- perform component processing
if _OPTIONS['game'] == 'dummy' then
	return
end

root_cwd = os.getcwd()

-- initialize components
dofile('components/config.lua')
dofile('vendor/config.lua')

component
{
	name = 'platform:' .. os.get(),
	rawName = os.get(),
	vendor = { dummy = true }
}

solution "CitizenMP"
	configurations { "Debug", "Release" }

	flags { "No64BitChecks", "Symbols", "Unicode" }
	
	flags { "NoIncrementalLink", "NoEditAndContinue" } -- this breaks our custom section ordering in citilaunch, and is kind of annoying otherwise
	
	includedirs { "shared/", "client/shared/", "../vendor/jitasm/", "deplibs/include/", "../vendor/gmock/include/", "../vendor/gtest/include", os.getenv("BOOST_ROOT") }
	
	defines { "GTEST_HAS_PTHREAD=0", "BOOST_ALL_NO_LIB" }

	libdirs { "deplibs/lib/" }

	location ("build/" .. _OPTIONS['game'])

	if _OPTIONS['game'] == 'server' then
		location ("build/server/" .. os.get())
	end
	
	configuration "Debug*"
		targetdir ("bin/" .. _OPTIONS['game'] .. "/debug")
		defines "NDEBUG"

		defines { '_ITERATOR_DEBUG_LEVEL=0' }

		if _OPTIONS['game'] == 'server' then
			targetdir ("bin/server/" .. os.get() .. "/debug")
		end
		
	configuration "Release*"
		targetdir ("bin/" .. _OPTIONS['game'] .. "/release")
		defines "NDEBUG"
		optimize "Speed"

		if _OPTIONS['game'] == 'server' then
			targetdir ("bin/server/" .. os.get() .. "/release")
		end
		
	configuration "game=ny"
		defines "GTA_NY"

	configuration "game=payne"
		defines "PAYNE"

	configuration "windows"
		links { "winmm" }

	configuration "not windows"
		buildoptions {
			"-fPIC", -- required to link on AMD64
		}

		links { "c++" }

	-- TARGET: launcher
	if _OPTIONS['game'] ~= 'server' then
		-- game launcher
		dofile 'client/launcher/build.lua'
	else
		dofile 'server/launcher/build.lua'
	end

	project "CitiCore"
		targetname "CoreRT" 
		language "C++"
		kind "SharedLib"

		files
		{
			"client/citicore/**.cpp", "client/citicore/**.h", "client/common/Error.cpp", "client/common/StdInc.cpp"
		}

		links { "Shared" }

		defines "COMPILING_CORE"

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"

		configuration "not windows"
			links { "dl", "c++" }

if _OPTIONS['game'] ~= 'server' then
	project "CitiGame"
		targetname "CitizenGame"
		language "C++"
		kind "SharedLib"
		
		files
		{
			"client/citigame/**.cpp", "client/citigame/**.h", "client/common/Error.cpp", "client/citigame/**.c", "client/common/StdInc.cpp"
		}
		
		links { "Shared", "citicore" }
		
		defines "COMPILING_GAME"
		
		includedirs { "client/citigame/include/", "components/nui-core/include/", "components/downloadmgr/include/", "components/net/include/", "client/citicore/", "components/resources/include/", "components/http-client/include/", "../vendor/luajit/src/", "../vendor/yaml-cpp/include/", "../vendor/msgpack-c/include/", "deplibs/include/msgpack-c/", "client/libcef/", "client/shared/np" }
		
		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
		
		configuration "game=ny"
			includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/citigame/net/" }
			links { "HooksNY", "GameNY" }

			-- temp, until CitiGame is gone
			links { "rage-nutsnbolts-" .. _OPTIONS['game'], "http-client", "net", "resources", "downloadmgr", "nui-core" }

			includedirs { "components/rage-nutsnbolts-" .. _OPTIONS['game'] .. "/include/" }
end

	local buildHost = os.getenv("COMPUTERNAME") or 'dummy'

	--[[if buildHost == 'FALLARBOR' then
		project "CitiMono"
			targetname "CitizenFX.Core"
			language "C#"
			kind "SharedLib"

			files { "client/clrcore/**.cs" }

			links { "System" }

			configuration "Debug*"
				targetdir "bin/debug/citizen/clr/lib/mono/4.5"

			configuration "Release*"
				targetdir "bin/release/citizen/clr/lib/mono/4.5"
	end]]

group "managed"

if _OPTIONS['game'] ~= 'server' and buildHost == 'FALLARBOR' then
	external 'CitiMono'
		uuid 'E781BFF9-D34E-1A05-FC67-08ADE8934F93'
		kind 'SharedLib'
		language 'C#'
		location '.'

	external '010.Irony.2010'
		uuid 'D81F5C91-D7DB-46E5-BC99-49488FB6814C'
		kind 'SharedLib'
		language 'C#'
		location '../vendor/pash/Libraries/Irony/Irony/'

	external 'System.Management'
		uuid 'C5E303EC-5684-4C95-B0EC-2593E6662403'
		kind 'SharedLib'
		language 'C#'
		location '../vendor/pash/Source/System.Management/'

	external 'Microsoft.PowerShell.Commands.Utility'
		uuid '0E1D573C-C57D-4A83-A739-3A38E719D87E'
		kind 'SharedLib'
		language 'C#'
		location '../vendor/pash/Source/Microsoft.PowerShell.Commands.Utility/'
end
			
	if _OPTIONS['game'] == 'ny' then
		project "GameNY"
			targetname "game_ny"
			language "C++"
			kind "SharedLib"
			
			links { "Shared", "zlib", "CitiCore" }
			
			defines "COMPILING_GAMESPEC"
			
			pchsource "client/common/StdInc.cpp"
			pchheader "StdInc.h"
			
			configuration "game=ny"
				includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "../vendor/zlib/" }
			
				files
				{
					"client/game_ny/**.cpp", "client/game_ny/**.h", "client/common/Error.cpp", "client/common/StdInc.cpp"
				}
			
		project "HooksNY"
			targetname "hooks_ny"
			language "C++"
			kind "SharedLib"
			
			links { "Shared", "GameNY", "ws2_32", "rage-graphics-ny", "gta-core-ny", "CitiCore" }
			
			defines "COMPILING_HOOKS"	

			pchsource "client/hooks_ny/base/StdInc.cpp"
			pchheader "StdInc.h"		
			
			configuration "game=ny"
				includedirs { "components/gta-core-ny/include", "components/rage-graphics-ny/include", "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/", "client/hooks_ny/base/" }
				
				files
				{
					"client/hooks_ny/**.cpp", "client/hooks_ny/**.h", "client/common/Error.cpp"
				}
	end
		
	group ""

	project "Shared"
		targetname "shared"
		language "C++"
		kind "StaticLib"

		defines "COMPILING_SHARED"
		
		--includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/" }
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}

		configuration "not windows"
			excludes { "**/Hooking.*" }

	project "SharedLibc"
		targetname "shared_libc"
		language "C++"
		kind "StaticLib"

		flags { "StaticRuntime" }

		defines { "COMPILING_SHARED", "COMPILING_SHARED_LIBC" }
		
		--includedirs { "client/game_ny/base/", "client/game_ny/gta/", "client/game_ny/rage/" }
		
		files
		{
			"shared/**.cpp", "shared/**.h", "client/shared/**.cpp", "client/shared/**.h"
		}

		configuration "not windows"
			excludes { "**/Hooking.*" }

	group "vendor"
		
if _OPTIONS['game'] ~= 'server' then
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
end
		
	project "gtest_main"
		language "C++"
		kind "StaticLib"

		includedirs { "../vendor/gtest/" }

		files { "../vendor/gtest/src/gtest-all.cc", "../vendor/gtest/src/gtest_main.cc" }

	project "gmock_main"
		language "C++"
		kind "StaticLib"
		
		includedirs { "../vendor/gmock/", "../vendor/gtest/" }
		files { "../vendor/gmock/src/gmock-all.cc", "../vendor/gmock/src/gmock_main.cc" }

if _OPTIONS['game'] ~= 'server' then
	project "tests_citigame"
		language "C++"
		kind "ConsoleApp"
		
		links { "gmock_main", "gtest_main", "CitiGame", "CitiCore", "Shared" }
		
		includedirs { "client/citigame/include/", "client/citicore/" }
		
		files { "tests/citigame/*.cpp", "tests/test.cpp" }
end

	-- run components
	group "components"
		do_components()

	-- vendor is last so it can trigger even if it were merely tagged
	group "vendor"
		do_vendor()
end, function(e)
	print(e)
	print(debug.traceback())

	os.exit(0)
end)