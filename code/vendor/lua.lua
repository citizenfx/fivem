return {
	include = function()
		includedirs { "vendor/lua/src" }
		includedirs { "../vendor/lua-cmsgpack/src" }
		includedirs { "../vendor/lua-rapidjson/src" }

		add_dependencies 'vendor:msgpack-c'
	end,

	run = function()
		targetname "lua"
		language "C"
		kind "StaticLib"
		
		if os.istarget('windows') then
			flags { "LinkTimeOptimization" }
		end

		defines {
			'LUACMSGPACK_COMPAT',
			'LUA_RAPIDJSON_COMPAT',
			'LUA_RAPIDJSON_SANITIZE_KEYS',
		}

		files
		{
			"vendor/lua/src/*.c",
			"../vendor/lua-cmsgpack/src/*.c",
			"../vendor/lua-cmsgpack/src/*.h",
			"../vendor/lua-rapidjson/src/*.h",
			"../vendor/lua-rapidjson/src/*.hpp",
			"../vendor/lua-rapidjson/src/*.cpp"
		}

		removefiles {
			"vendor/lua/src/lua.c", "vendor/lua/src/luac.c"
		}
	end
}