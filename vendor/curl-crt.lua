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

		defines { 'CURL_STATICLIB', 'BUILDING_LIBCURL', 'USE_IPV6', 'HTTP_ONLY', 'USE_IPV6', 'USE_WINDOWS_SSPI', 'USE_SCHANNEL' }

		prebuildcommands
		{
			-- sadly premake's {COPY} breaks on Windows (xcopy error 'Cannot perform a cyclic copy')
			'copy /y %{prj.location}\\..\\..\\..\\vendor\\curl\\include\\curl\\curlbuild.h.dist %{prj.location}\\..\\..\\..\\vendor\\curl\\include\\curl\\curlbuild.h'
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