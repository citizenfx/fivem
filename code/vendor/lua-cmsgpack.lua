local a = ...

return {
	include = function()
		includedirs { "../vendor/lua-cmsgpack" }

		add_dependencies 'vendor:msgpack-c'
		add_dependencies 'vendor:lua'
	end,

	run = function()
		targetname "lua-cmsgpack"
		language "C"
		kind "StaticLib"
		if a then
			staticruntime "On"
		end		

        defines { 
			'GRIT_POWER_TTYPE',
        }

		files { 
			"../vendor/lua-cmsgpack/*.c",
			"../vendor/lua-cmsgpack/*.h",
		}
	end
}
