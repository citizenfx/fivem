return {
	include = function()
		includedirs "../vendor/mbedtls/include/"
	end,
	
	run = function()
		language "C"
		kind "StaticLib"
		
		defines {
			"SODIUM_STATIC",
			"NATIVE_LITTLE_ENDIAN"
		}
		
		files_project "../vendor/mbedtls/" {
			"library/**.c"
		}
	end
}