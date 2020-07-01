return {
	include = function()
		includedirs { "vendor/lua/src" }
		includedirs { "../vendor/lua-cmsgpack" }
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

		defines { 'GRIT_POWER_TTYPE' }

		files
		{
			"vendor/lua/src/*.c",
			"../vendor/lua-cmsgpack/*.c",
			"../vendor/lua-cmsgpack/*.h",
			"../vendor/lua-rapidjson/src/*.h",
			"../vendor/lua-rapidjson/src/*.hpp",
			"../vendor/lua-rapidjson/src/*.cpp"			
		}

		removefiles {
			"vendor/lua/src/lua.c", "vendor/lua/src/luac.c"
		}
	end
}