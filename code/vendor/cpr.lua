return {
	include = function()
		includedirs "../vendor/cpr/include/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		includedirs { "../vendor/curl/include/" }

		defines { 'CURL_STATICLIB' }

		files {
			"../vendor/cpr/cpr/*.cpp",
		}
	end
}
