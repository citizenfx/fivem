return {
	include = function()
		includedirs "../vendor/linenoise-ng/include/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files {
			"../vendor/linenoise-ng/src/*.cpp",
		}
	end
}
