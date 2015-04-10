return {
	run = function()
		language "C++"
		kind 'StaticLib'

		files_project(os.getenv("BOOST_ROOT") .. '/libs/system/src/')
		{
			'error_code.cpp'
		}

		files_project(os.getenv("BOOST_ROOT") .. '/libs/filesystem/src/')
		{
			'codecvt_error_category.cpp',
			'operations.cpp',
			'path.cpp',
			'path_traits.cpp',
			'portability.cpp',
			'unique_path.cpp',
			'utf8_codecvt_facet.cpp',
			'windows_file_codecvt.cpp',
		}
	end
}