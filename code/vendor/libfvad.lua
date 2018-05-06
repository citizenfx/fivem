return {
	include = function()
		includedirs "../vendor/libfvad/include/"
	end,
	
	run = function()
		language 'C'
		kind 'StaticLib'
		
		files_project '../vendor/libfvad/src/'  {
			"fvad.c",
			"signal_processing/division_operations.c",
			"signal_processing/energy.c",
			"signal_processing/get_scaling_square.c",
			"signal_processing/resample_48khz.c",
			"signal_processing/resample_by_2_internal.c",
			"signal_processing/resample_fractional.c",
			"signal_processing/spl_inl.c",
			"vad/vad_core.c",
			"vad/vad_filterbank.c",
			"vad/vad_gmm.c",
			"vad/vad_sp.c",
		}
	end
}