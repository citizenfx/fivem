return {
	include = function()
		includedirs { "vendor/pplx/include/", "../vendor/thread-pool-cpp/include/" }
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		includedirs { "vendor/pplx/include/", "../vendor/thread-pool-cpp/include/" }

		files_project 'vendor/pplx/src/' {
			'pplx.cpp',
			'stdafx.cpp',
		}

		filter 'system:linux'
			files_project 'vendor/pplx/src/' {
				'pplxlinux.cpp'
			}

		filter 'system:windows'
			files_project 'vendor/pplx/src/' {
				'pplxwin.cpp'
			}
	end
}
