return {
	include = function()
		includedirs { "../vendor/minizip/" }
		
		add_dependencies 'vendor:zlib'
	end,

	run = function()
		targetname "minizip"
		
		language "C"
		kind "StaticLib"
		
		defines { "HAVE_ZLIB" }

		flags "NoRuntimeChecks"

		files_project '../vendor/minizip/'
		{
			'mz_strm.c',
			'mz_strm_mem.c',
			'mz_strm_crc32.c',
			'mz_strm_zlib.c',
			'mz_zip.c',
			'mz_zip_rw.c',
		}
	end
}
