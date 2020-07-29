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
			'LUACMSGPACK_COMPAT',
		}

		files { 
			"../vendor/lua-cmsgpack/src/*.c",
			"../vendor/lua-cmsgpack/src/*.h",
		}
	end
}
