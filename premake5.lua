solution "CitizenMP"
	configurations { "Debug NY", "Release NY" }
	
	flags { "StaticRuntime", "No64BitChecks", "Symbols", "Unicode" }
	
	flags { "NoIncrementalLink", "NoEditAndContinue" } -- this breaks our custom section ordering in citilaunch, and is kind of annoying otherwise
	
	includedirs { "shared/", "client/shared/", "../vendor/jitasm/", "deplibs/include/", "../vendor/gtest/include/", os.getenv("BOOST_ROOT") }
	
	defines { "GTEST_HAS_PTHREAD=0" }
	
	libdirs { "deplibs/lib/" }

	links { "winmm" }
	
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

	project "CitiGame"
		targetname "CitizenGame"
		language "C++"
		kind "SharedLib"
		
		files
		{
			"client/citigame/**.cpp", "client/citigame/**.h", "client/common/Error.cpp", "client/citigame/**.c", "client/common/StdInc.cpp"
		}
		
		links { "Shared", "citicore", "yaml-cpp", "msgpack-c", "lua51", "winmm", "winhttp", "ws2_32", "libcef_dll", "libcef", "delayimp", "libnp", "http-client", "net", "resources", "downloadmgr" }
		
		defines "COMPILING_GAME"
		
		libdirs { "../vendor/luajit/src/", "client/libcef/lib/", "client/shared/np" }
		includedirs { "client/citigame/include/", "components/downloadmgr/include/", "components/net/include/", "client/citicore/", "components/resources/include/", "components/http-client/include/", "../vendor/luajit/src/", "../vendor/yaml-cpp/include/", "../vendor/msgpack-c/src/", "deplibs/include/msgpack-c/", "client/libcef/", "client/shared/np" }
		
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

		defines "COMPILING_SHARED"
		
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
		
		links { "gtest_main", "CitiGame", "CitiCore", "Shared" }
		
		includedirs { "client/citigame/include/", "client/citicore/" }
		
		files { "tests/citigame/*.cpp", "tests/test.cpp" }

	-- code for component development
	local components = { }

	dependency = function(name)
		-- find a matching component
		--[[local cname

		for _, c in ipairs(components) do
			if c == name then
				cname = c
				break
			else
				local basename = c:gsub('(.+)-ny', '%1')

				if basename == name then
					cname = c
					break
				end
			end
		end

		if not cname then
			error("Component " .. name .. " seems unknown.")
		end

		includedirs { '../' .. name .. '/include/' }

		links { name }]]

		return
	end

	package.path = '?.lua'

	local json = require('json')

	component = function(name)
		local filename = name .. '/component.json'

		io.input(filename)
		local jsonStr = io.read('*all')
		io.close()

		local decoded = json.decode(jsonStr)

		decoded.rawName = name

		table.insert(components, decoded)
	end

	local do_component = function(name, comp)
		project(name)

		language "C++"
		kind "SharedLib"

		includedirs { "client/citicore/", 'components/' .. name .. "/include/" }
		files {
			'components/' .. name .. "/src/**.cpp",
			'components/' .. name .. "/src/**.h",
			'components/' .. name .. "/include/**.h",
			'components/' .. name .. "/component.rc",
			"client/common/StdInc.cpp",
			"client/common/Error.cpp"
		}

		defines { "COMPILING_" .. name:upper():gsub('-', '_'), 'HAS_LOCAL_H' }

		links { "Shared", "CitiCore" }

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"

		dofile('components/' .. name .. '/component.lua')

		-- do automatic dependencies
		if not comp.dependencies then
			comp.dependencies = {}
		end

		local function id_matches(full, partial)
			local tokenString = ''
			local partialTemp = partial .. ':'

			for token in string.gmatch(full:gsub('\\[.+\\]', ''), '[^:]+') do
				tokenString = tokenString .. token .. ':'

				if partialTemp == tokenString then
					return true
				end
			end

			return false
		end

		local function find_match(id)
			for _, mcomp in ipairs(components) do
				if mcomp.name == id then
					return mcomp
				end

				if id_matches(mcomp.name, id) then
					return mcomp
				end
			end

			return nil
		end

		local function process_dependencies(comp)
			for _, dep in ipairs(comp.dependencies) do
				-- find a match for the dependency
				local match = find_match(dep)

				if match then
					print(match.name, 'matches', dep)

					dofile('components/' .. match.rawName .. '/component.lua')

					includedirs { 'components/' .. match.rawName .. '/include/' }

					links { match.rawName }

					process_dependencies(match)
				end
			end
		end

		process_dependencies(comp)

		-- test project
		project('tests_' .. name)

		language "C++"
		kind "ConsoleApp"

		includedirs { 'components/' .. name .. "/include/" }
		files { 'components/' .. name .. "/tests/**.cpp", 'components/' .. name .. "/tests/**.h", "tests/test.cpp", "client/common/StdInc.cpp" }

		links { "Shared", "CitiCore", "gtest_main", name }

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
	end

	dofile('components/config.lua')

	for _, comp in ipairs(components) do
		do_component(comp.rawName, comp)
	end