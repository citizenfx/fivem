return {
	run = function()
		language "C++"
		kind 'StaticLib'

		files_project('../vendor/boost-submodules/boost-program-options/src/')
		{
			'cmdline.cpp',
			'config_file.cpp',
			'convert.cpp',
			'options_description.cpp',
			'parsers.cpp',
			'positional_options.cpp',
			'split.cpp',
			'utf8_codecvt_facet.cpp',
			'value_semantic.cpp',
			'variables_map.cpp'
		}

		filter 'system:windows'
			files_project('../vendor/boost-submodules/boost-program-options/src/')
			{
				'winmain.cpp'
			}
	end
}