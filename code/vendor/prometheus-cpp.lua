return {
	include = function()
		includedirs { "vendor/prometheus-cpp/include/", "../vendor/prometheus-cpp/core/include/" }
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		files_project '../vendor/prometheus-cpp/core/'
		{
			'src/check_names.cc',
			'src/counter.cc',
			'src/detail/builder.cc',
			'src/detail/ckms_quantiles.cc',
			'src/detail/time_window_quantiles.cc',
			'src/detail/utils.cc',
			'src/family.cc',
			'src/gauge.cc',
			'src/histogram.cc',
			'src/registry.cc',
			'src/serializer.cc',
			'src/summary.cc',
			'src/text_serializer.cc',
		}
	end
}