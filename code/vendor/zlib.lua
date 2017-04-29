return {
	include = function()
		includedirs { "../vendor/zlib/" }
	end,

	run = function()
		targetname "zlib"
		language "C"
		kind "StaticLib"

		files { "../vendor/zlib/*.c", "../vendor/zlib/*.h" }
		excludes { "../vendor/zlib/example.c", "../vendor/zlib/minigzip.c" }

		configuration "windows"
			defines { "WIN32" }
	end
}