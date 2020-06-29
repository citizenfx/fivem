return {
	include = function()
		includedirs { "vendor/lua/src" }
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
			"vendor/lua/src/*.c" 
		}

		removefiles {
			"vendor/lua/src/lua.c", "vendor/lua/src/luac.c"
		}
	end
}