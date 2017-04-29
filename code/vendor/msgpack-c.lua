return {
	include = function()
		includedirs { "../vendor/msgpack-c/src", "../vendor/msgpack-c/include/", "deplibs/include/msgpack-c/" }
	end,

	run = function()
		targetname "msgpack-c"
		language "C++"
		kind "StaticLib"

		files
		{
			"../vendor/msgpack-c/src/*.c" 
		}
	end
}