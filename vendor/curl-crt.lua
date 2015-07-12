return {
	include = function()
		includedirs { "../vendor/curl/include/" }

		links { 'ws2_32' }
	end,

	run = function()
		targetname "curl-crt"
		language "C"
		kind "StaticLib"
		flags "StaticRuntime"

		-- all the disables except http/file
		defines { 'CURL_STATICLIB', 'BUILDING_LIBCURL', 'USE_IPV6', 'CURL_DISABLE_TFTP', 'CURL_DISABLE_FTP', 'CURL_DISABLE_LDAP', 'CURL_DISABLE_TELNET',
				  'CURL_DISABLE_DICT', 'CURL_DISABLE_RTSP', 'CURL_DISABLE_POP3', 'CURL_DISABLE_IMAP', 'CURL_DISABLE_SMTP', 
				  'CURL_DISABLE_RTMP', 'CURL_DISABLE_GOPHER', 'CURL_DISABLE_SMB', 'USE_IPV6', 'USE_WINDOWS_SSPI', 'USE_SCHANNEL' }

		prebuildcommands
		{
			-- sadly premake's {COPY} breaks on Windows (xcopy error 'Cannot perform a cyclic copy')
			'copy /y "%{prj.location}\\..\\..\\..\\vendor\\curl\\include\\curl\\curlbuild.h.dist" "%{prj.location}\\..\\..\\..\\vendor\\curl\\include\\curl\\curlbuild.h"'
		}

		includedirs {
			"../vendor/curl/lib/",
			"../vendor/curl/include/",

		}
		
		files_project '../vendor/curl/lib/'
		{
			'**.c'
		}
	end
}
