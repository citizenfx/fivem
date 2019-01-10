return {
	include = function()
		includedirs { "../vendor/nghttp2/lib/includes/" }

		defines { '_SSIZE_T_DEFINED=1', '_U_=', 'NGHTTP2_STATICLIB' }
		
		if os.istarget('windows') then
			defines 'ssize_t=__int64'
		else
			defines 'ssize_t=long'
		end
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
