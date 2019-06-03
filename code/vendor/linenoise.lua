return {
	include = function()
		includedirs "../vendor/replxx/include/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files {
			"../vendor/replxx/src/*.cpp",
			"../vendor/replxx/src/*.cxx",
		}
	end
}
