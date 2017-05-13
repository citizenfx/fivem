return {
	include = function()
		includedirs "../vendor/cpprestsdk/Release/include/"

		defines { 'CPPREST_FORCE_PPLX', '_NO_PPLXIMP', '_NO_ASYNCRTIMP' }
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		includedirs "../vendor/cpprestsdk/Release/src/pch/"

		files_project '../vendor/cpprestsdk/Release/src/' {
			'pplx/pplx.cpp',
			'pplx/threadpool.cpp',
			'pch/stdafx.cpp',
			'utilities/asyncrt_utils.cpp'
		}

		filter 'system:linux'
			files_project '../vendor/cpprestsdk/Release/src/' {
				'pplx/pplxlinux.cpp'
			}

		filter 'system:windows'
			files_project '../vendor/cpprestsdk/Release/src/' {
				'pplx/pplxwin.cpp'
			}
	end
}
