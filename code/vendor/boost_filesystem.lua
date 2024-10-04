return {
	include = function()
		add_dependencies { 'vendor:boost_system' }
	end,

	run = function()
		language "C++"
		kind 'StaticLib'
		
		defines { "BOOST_FILESYSTEM_NO_CXX20_ATOMIC_REF" }

		files_project('../vendor/boost-submodules/boost-filesystem/src/')
		{
			'codecvt_error_category.cpp',
			'directory.cpp',
			'exception.cpp',
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
