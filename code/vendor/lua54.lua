return {
	include = function()
		includedirs { "../vendor/lua/" }
		includedirs { "../vendor/lua-cmsgpack/src" }
		includedirs { "../vendor/lua-rapidjson/src" }
		
		add_dependencies 'vendor:msgpack-c'
	end,

	run = function()
		targetname "lua54"
		language "C"
		kind "StaticLib"
		
		if os.istarget('windows') then
			flags { "LinkTimeOptimization" }
		end

		defines { 
			'LUA_COMPAT_5_3', 
			'LUA_C99_MATHLIB',
			'GRIT_POWER_COMPOUND',
			'GRIT_POWER_INTABLE',
			'GRIT_POWER_TABINIT',
			'GRIT_POWER_SAFENAV',
			'GRIT_POWER_CCOMMENT',
			'GRIT_POWER_ANONDO',
			'GRIT_DEFER',
			'GRIT_POWER_OITER',
			'GRIT_COMPAT_IPAIRS',
			'GRIT_POWER_TCREATE',
			'GRIT_POWER_TTYPE',

			'LUACMSGPACK_COMPAT',
			-- 'LUA_RAPIDJSON_COMPAT', -- Lua54: strict JSON compliance required?
			'LUA_RAPIDJSON_SANITIZE_KEYS',			
		}

		files
		{
			"../vendor/lua/*.c",
			"../vendor/lua/*.h",
			"../vendor/lua-cmsgpack/src/*.c",
			"../vendor/lua-cmsgpack/src/*.h",
			"../vendor/lua-rapidjson/src/*.h",
			"../vendor/lua-rapidjson/src/*.hpp",
			"../vendor/lua-rapidjson/src/*.cpp"
		}

		removefiles {
			"../vendor/lua/lua.c", 
			"../vendor/lua/luac.c",
			"../vendor/lua/onelua.c",
		}
	end
}