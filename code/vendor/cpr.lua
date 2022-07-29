local a = ...

return {
	include = function()
		includedirs { "../vendor/cpr/include/", "vendor/cpr/include/" }
		
		if not os.istarget('windows') then
			linkoptions { '/usr/lib/libcurl.so' }
		end
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		includedirs { "../vendor/curl/include/" }

		defines { 'CURL_STATICLIB' }

		if a then
			staticruntime 'On'
		end

		files {
			"../vendor/cpr/cpr/*.cpp",
		}
	end
}
