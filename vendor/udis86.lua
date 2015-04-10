return {
	include = function()
		includedirs "../vendor/udis86/"
	end,

	run = function()
		language "C"
		kind "StaticLib"

		files {
			"../vendor/udis86/libudis86/*.c",
			"../vendor/udis86/libudis86/*.h",
		}
	end
}