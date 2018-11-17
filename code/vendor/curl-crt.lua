local a = ...

return {
	include = function()
		if os.istarget('windows') then
			includedirs { "../vendor/curl/include/" }

			links { 'ws2_32', 'crypt32' }
		else
			links { 'curl' }
		end
	end,

	run = function()
		if not os.istarget('windows') then
			targetname 'curl_dummy'
			language 'C'
			kind 'StaticLib'
			return
		end

		if a then
			targetname "curl"
		else
			targetname "curl-crt"
		end

		language "C"
		kind "StaticLib"

		flags "NoRuntimeChecks"

		if not a then
			staticruntime 'On'
		end

		-- nghttp2
		add_dependencies 'vendor:nghttp2'

		-- all the disables except http/file
		defines { 'CURL_STATICLIB', 'BUILDING_LIBCURL', 'USE_IPV6', 'CURL_DISABLE_TFTP', 'CURL_DISABLE_FTP', 'CURL_DISABLE_LDAP', 'CURL_DISABLE_TELNET',
				  'CURL_DISABLE_DICT', 'CURL_DISABLE_RTSP', 'CURL_DISABLE_POP3', 'CURL_DISABLE_IMAP', 'CURL_DISABLE_SMTP',
				  'CURL_DISABLE_RTMP', 'CURL_DISABLE_GOPHER', 'CURL_DISABLE_SMB', 'USE_IPV6', 'USE_NGHTTP2' }

		if os.istarget('windows') then
			defines { 'USE_WINDOWS_SSPI', 'USE_SCHANNEL' }
		else
			defines { 'USE_OPENSSL' }
		end

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
