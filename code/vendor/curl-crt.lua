local a = ...

return {
	include = function()
		includedirs { "../vendor/curl/include/" }

		links { 'ws2_32', 'crypt32' }
	end,

	run = function()
		if a then
			targetname "curl"
		else
			targetname "curl-crt"
		end

		language "C"
		kind "StaticLib"

		flags "NoRuntimeChecks"

		if not a then
			flags "StaticRuntime"
		end

		-- nghttp2
		includedirs { "../vendor/nghttp2/lib/includes/" }

		defines { 'ssize_t=long', '_SSIZE_T_DEFINED=1', '_U_=', 'NGHTTP2_STATICLIB' }

		-- all the disables except http/file
		defines { 'CURL_STATICLIB', 'BUILDING_LIBCURL', 'USE_IPV6', 'CURL_DISABLE_TFTP', 'CURL_DISABLE_FTP', 'CURL_DISABLE_LDAP', 'CURL_DISABLE_TELNET',
				  'CURL_DISABLE_DICT', 'CURL_DISABLE_RTSP', 'CURL_DISABLE_POP3', 'CURL_DISABLE_IMAP', 'CURL_DISABLE_SMTP', 
				  'CURL_DISABLE_RTMP', 'CURL_DISABLE_GOPHER', 'CURL_DISABLE_SMB', 'USE_IPV6', 'USE_WINDOWS_SSPI', 'USE_SCHANNEL', 'USE_NGHTTP2' }

		prebuildcommands
		{
			-- sadly premake's {COPY} breaks on Windows (xcopy error 'Cannot perform a cyclic copy')
			(
				'copy /y "%s\\vendor\\curl\\include\\curl\\curlbuild.h.dist" "%s\\vendor\\curl\\include\\curl\\curlbuild.h"'
			):format(
				path.getabsolute('../'),
				path.getabsolute('../')
			)
		}

		includedirs {
			"../vendor/curl/lib/",
			"../vendor/curl/include/",

		}
		
		files_project '../vendor/curl/lib/'
		{
			'**.c'
		}

		files_project '../vendor/nghttp2/lib/'
		{
			'*.c'
		}
	end
}
