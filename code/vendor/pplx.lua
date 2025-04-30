return {
	include = function()
		includedirs { "vendor/pplx/include/" }

		defines { '_NO_PPLXIMP', '_NO_ASYNCRTIMP', '_PPLTASK_ASYNC_LOGGING=0' }
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		defines { "CPPREST_EXCLUDE_WEBSOCKETS" }

		includedirs { "vendor/pplx/include/" }

		files_project 'vendor/pplx/src/' {
			'pplx.cpp',
			'stdafx.cpp',
			'threadpool.cpp'
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
