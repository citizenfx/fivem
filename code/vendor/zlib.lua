local a = ...

return {
	include = function()
		includedirs { "../vendor/zlib/" }
	end,

	run = function()
		language "C"
		kind "StaticLib"
		
		if a then
			staticruntime "On"
		end

		files { "../vendor/zlib/*.c", "../vendor/zlib/*.h" }
		excludes { "../vendor/zlib/example.c", "../vendor/zlib/minigzip.c" }

		configuration "windows"
			defines { "WIN32" }
	end
}