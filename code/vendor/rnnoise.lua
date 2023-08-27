return {
	include = function()
		includedirs "../vendor/rnnoise/include/"
	end,

	run = function()
		language 'C'
		kind 'StaticLib'

		defines { 'USE_MALLOC', '_USE_MATH_DEFINES' }

		for _, v in pairs({
			'opus_fft_c',
			'opus_ifft_c',
			'opus_fft_impl',
			'compute_dense',
			'compute_gru',
		}) do
			defines { ('%s=rnnoise_%s'):format(v, v) }
		end

		files_project '../vendor/rnnoise/src/'  {
			'**.c',
			'**.h',
		}
	end
}
