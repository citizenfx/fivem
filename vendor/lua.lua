return {
	include = function()
		includedirs { "vendor/lua/src" }
	end,

	run = function()
		targetname "lua"
		language "C"
		kind "StaticLib"

		files
		{
			"vendor/lua/src/*.c" 
		}

		removefiles {
			"vendor/lua/src/lua.c", "vendor/lua/src/luac.c"
		}
	end
}