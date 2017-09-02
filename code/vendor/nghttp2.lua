return {
	include = function()
		includedirs { "../vendor/nghttp2/lib/includes/" }

		defines { 'ssize_t=long', '_SSIZE_T_DEFINED=1', '_U_=', 'NGHTTP2_STATICLIB' }
	end,

	run = function()
		targetname "nghttp2"

		language "C"
		kind "StaticLib"

		flags "NoRuntimeChecks"

		files_project '../vendor/nghttp2/lib/'
		{
			'*.c'
		}
	end
}
