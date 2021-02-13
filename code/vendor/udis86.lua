return {
	include = function()
		includedirs "../vendor/udis86/"
	end,

	run = function()
		language "C"
		kind "StaticLib"

		dependson 'CfxPrebuild'

		files {
			"../vendor/udis86/libudis86/*.c",
			"../vendor/udis86/libudis86/*.h",

			"../vendor/udis86/libudis86/itab.c",
			"../vendor/udis86/libudis86/itab.h",
		}
	end
}