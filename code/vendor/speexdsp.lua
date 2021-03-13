return {
	include = function()
		includedirs {
			'../vendor/speexdsp/include/',
			'../vendor/speexdsp/include/speex/',
		}
	end,

	run = function()
		language "C"
		kind "StaticLib"

		defines { 'USE_ALLOCA', 'inline=__inline', 'FLOATING_POINT', 'USE_SMALLFT', 'EXPORT=' }

		if os.istarget('windows') then
			defines { 'WIN32' }
		end

		files_project '../vendor/speexdsp/libspeexdsp/' {
			'preprocess.c',
			'jitter.c',
			'mdf.c',
			'fftwrap.c',
			'filterbank.c',
			'resample.c',
			'buffer.c',
			'scal.c',
			'smallft.c',
		}

	end
}