return {
	include = function()
		includedirs "../vendor/cpu_features/include/"
	end,

	run = function()
		language "C"
		kind "StaticLib"

		defines { "STACK_LINE_READER_BUFFER_SIZE=1024" }

		files_project "../vendor/cpu_features/" {
			"src/**.c",
		}

		if not os.istarget('linux') then
			removefiles "../vendor/cpu_features/src/hwcaps.c"
		else
			defines {
				"HAVE_STRONG_GETAUXVAL",
				"HAVE_DLFCN_H"
			}
		end
	end
}
