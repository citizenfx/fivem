return {
	run = function()
		language "C++"
		kind 'StaticLib'

		files_project(os.getenv("BOOST_ROOT") .. '/libs/program_options/src/')
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
			files_project(os.getenv("BOOST_ROOT") .. '/libs/program_options/src/')
			{
				'winmain.cpp'
			}
	end
}