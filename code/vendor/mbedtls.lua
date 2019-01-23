return {
	include = function()
		includedirs "../vendor/mbedtls/include/"
	end,
	
	run = function()
		language "C"
		kind "StaticLib"
		
		defines {
			"NATIVE_LITTLE_ENDIAN",
			"MBEDTLS_HAVEGE_C"
		}
		
		files_project "../vendor/mbedtls/" {
			"library/**.c"
		}
	end
}