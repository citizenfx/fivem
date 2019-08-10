return {
	include = function()
		includedirs "../vendor/cpp-url/include/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files {
			"../vendor/cpp-url/src/**.cpp",
		}
	end
}
