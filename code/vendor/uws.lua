return {
	include = function()
		includedirs '../vendor/uws/'
		includedirs '../vendor/uSockets/src/'
		
		add_dependencies 'vendor:libuv'
		add_dependencies 'vendor:zlib'
	end,

	run = function()
		targetname "uws"
		language "C++"
		kind "StaticLib"
		
		defines { "UWS_THREADSAFE", "LIBUS_NO_SSL" }

		flags "NoRuntimeChecks"

		files_project '../vendor/uws/src/'
		{
			'**.cpp',
			'**.h'
		}
		
		files_project '../vendor/uSockets/src/'
		{
			'**.c',
			'**.h'
		}
	end
}