return {
	include = function()
		includedirs { "vendor/lua/src" }
		includedirs { "../vendor/lua-cmsgpack/src" }
		includedirs { "../vendor/lua-rapidjson/src" }
		includedirs { "../vendor/lmprof/src" }

		add_dependencies 'vendor:msgpack-c'
	end,

	run = function()
		targetname "lua"
		language "C"
		kind "StaticLib"
		
        if os.istarget('windows') then
            flags { "LinkTimeOptimization" }
            defines { "LUA_BUILD_AS_DLL" }
        elseif os.istarget('linux') then
            defines { "LUA_USE_POSIX" }
        end

		defines {
			'LUACMSGPACK_COMPAT',
			'LUA_RAPIDJSON_COMPAT',
			'LUA_RAPIDJSON_SANITIZE_KEYS',

			--[[ Profiler Library --]]
			'LMPROF_BUILTIN', -- Use internal headers: lobject.h/lstate.h
			'LMPROF_HASH_SPLITMIX', -- Use splitmix record hashing
			'LMPROF_DISABLE_OUTPUT_PATH', -- Disable 'output_path' argument handling.
		}

		files
		{
			"vendor/lua/src/*.c",
			"../vendor/lua-cmsgpack/src/*.c",
			"../vendor/lua-cmsgpack/src/*.h",
			"../vendor/lua-rapidjson/src/*.h",
			"../vendor/lua-rapidjson/src/*.hpp",
			"../vendor/lua-rapidjson/src/*.cpp",
			"../vendor/lmprof/src/**",
		}

		removefiles {
			"vendor/lua/src/lua.c", "vendor/lua/src/luac.c"
		}
	end
}