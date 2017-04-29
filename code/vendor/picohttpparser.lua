return {
	include = function()
		includedirs { "../vendor/picohttpparser/" }
	end,

	run = function()
		language "C"
		kind "StaticLib"

		files_project '../vendor/picohttpparser/'
		{
			'picohttpparser.c',
			'picohttpparser.h'
		}
	end
}