local a, static = ...

return {
	include = function()
		if os.istarget('windows') then
			includedirs { "../vendor/curl/include/" }

			links { 'ws2_32', 'crypt32' }
		else
			links { 'curl' }
			
			linkoptions { '/usr/lib/libcurl.so' }
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
		elseif static then
			targetname "curl-static"
		else
			targetname "curl-crt"
		end

		language "C"

		flags "NoRuntimeChecks"

		if (not a) or static then
			if not a then
				staticruntime 'On'
			end
			
			kind "StaticLib"
			
			defines { 'CURL_STATICLIB' }
		else
			kind "SharedLib"
			targetname "cfx_curl_x86_64"
		end

		-- nghttp2
		add_dependencies 'vendor:nghttp2'

		-- all the disables except http/file
		defines { 'BUILDING_LIBCURL', 'USE_IPV6', 'CURL_DISABLE_TFTP', 'CURL_DISABLE_FTP', 'CURL_DISABLE_LDAP', 'CURL_DISABLE_TELNET',
				  'CURL_DISABLE_DICT', 'CURL_DISABLE_RTSP', 'CURL_DISABLE_POP3', 'CURL_DISABLE_IMAP', 'CURL_DISABLE_SMTP',
				  'CURL_DISABLE_RTMP', 'CURL_DISABLE_GOPHER', 'CURL_DISABLE_SMB', 'USE_IPV6', 'USE_NGHTTP2' }

		if os.istarget('windows') then
			defines { 'USE_WINDOWS_SSPI', 'USE_OPENSSL', 'OPENSSL_NO_ENGINE' }
			
			if a then
				add_dependencies 'vendor:openssl_crypto'
				add_dependencies 'vendor:openssl_ssl'
			else
				add_dependencies 'vendor:openssl_crypto_crt'
				add_dependencies 'vendor:openssl_ssl_crt'
			end
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
