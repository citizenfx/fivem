local dynamic = ...

return {
	include = function()
		includedirs { "../vendor/xz/src/liblzma/api/" }
	end,

	run = function()
		if dynamic then
			targetname "xz"
		else
			targetname "xz-crt"
		end

		language "C"
		kind "StaticLib"
		
		if not dynamic then
			staticruntime 'On'
		end

		defines { 'HAVE_STDINT_H=1', 'HAVE_CONFIG_H=1', 'HAVE_USABLE_CLMUL=1' }

		includedirs {
			"../vendor/xz/windows/",
			"../vendor/xz/src/common/",
			"../vendor/xz/src/liblzma/common/",
			"../vendor/xz/src/liblzma/check/",
			"../vendor/xz/src/liblzma/delta/",
			"../vendor/xz/src/liblzma/lz/",
			"../vendor/xz/src/liblzma/lzma/",
			"../vendor/xz/src/liblzma/rangecoder/",
			"../vendor/xz/src/liblzma/simple/",
			"../vendor/xz/windows/vs2019/",
		}
		
		files_project '../vendor/xz/src/liblzma/'
		{
			'**.c',
			'../common/tuklib_physmem.c'
		}

		removefiles
		{
			'../**/crc32_small.c', '../**/crc64_small.c', '../**/*_tablegen.c'
		}
	end
}