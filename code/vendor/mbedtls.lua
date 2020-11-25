local a = ...

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
		
		if a then
			staticruntime 'On'
		end
		
		files_project "../vendor/mbedtls/" {
			"library/**.c"
		}
	end
}