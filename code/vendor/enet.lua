return {
	include = function()
		includedirs "../vendor/enet/include/"

		links { 'ws2_32', 'winmm' }
	end,

	run = function()
		language "C"
		kind "StaticLib"

		defines { "HAS_INET_NTOP", "HAS_INET_PTON" }

		files_project "../vendor/enet/" {
			"*.c"
		}
	end
}
