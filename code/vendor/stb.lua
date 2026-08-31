return {
	include = function()
		includedirs "../vendor/stb/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files
		{
			"components/extra-natives-five/src/stb_image_write_impl.cpp",
		}
	end
}
