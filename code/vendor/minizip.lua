local a = ...

return {
	include = function()
		includedirs { "../vendor/minizip/" }
		
		if not a then
			add_dependencies 'vendor:zlib'
		else
			add_dependencies 'vendor:zlib-crt'
		end
	end,

	run = function()
		language "C"
		kind "StaticLib"
		
		defines { "HAVE_ZLIB" }

		flags "NoRuntimeChecks"
		
		if a then
			staticruntime 'On'
		end

		files_project '../vendor/minizip/'
		{
			'mz_strm.c',
			'mz_strm_mem.c',
			'mz_strm_crc32.c',
			'mz_strm_zlib.c',
			'mz_zip.c',
			'mz_zip_rw.c'
		}
		
		if os.istarget('windows') then
			files_project '../vendor/minizip/'
			{
				'mz_os.c',
				'mz_os_win32.c',
				'mz_strm_win32.c',
				'mz_strm_buf.c',
				'mz_strm_split.c',
			}
		end
	end
}
