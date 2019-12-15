return {
	include = function()
		includedirs "../vendor/toojpeg/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files_project '../vendor/toojpeg/' {
			'toojpeg.cpp'
		}
	end
}
