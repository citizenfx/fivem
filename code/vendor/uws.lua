return {
	include = function()
		includedirs '../vendor/uws/'
		
		add_dependencies 'vendor:libuv'
		add_dependencies 'vendor:openssl_crypto'
		add_dependencies 'vendor:openssl_ssl'
		add_dependencies 'vendor:zlib'
	end,

	run = function()
		targetname "uws"
		language "C++"
		kind "StaticLib"
		
		defines { "USE_LIBUV", "UWS_THREADSAFE" }

		flags "NoRuntimeChecks"

		files_project '../vendor/uws/src/'
		{
			'**.cpp',
			'**.h'
		}
	end
}