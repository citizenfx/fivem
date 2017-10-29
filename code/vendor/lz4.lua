return {
	include = function()
		includedirs "../vendor/lz4/lib/"
	end,

	run = function()
		language "C"
		kind "StaticLib"

		files_project '../vendor/lz4/lib/' {
			'lz4.c',
			'lz4frame.c',
			'lz4hc.c',
			'xxhash.c'
		}
	end
}
