return {
	include = function()
		includedirs "../vendor/enet/include/"

		links { 'ws2_32', 'winmm' }
	end,

	run = function()
		language "C"
		kind "StaticLib"

		files_project "../vendor/enet/" {
			"*.c"
		}
	end
}