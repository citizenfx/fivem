return {
	include = function()
		includedirs { "../vendor/libsodium/src/libsodium/include/", "vendor/libsodium/" }
	end,
	
	run = function()
		targetname "libsodium"
		language "C"
		kind "StaticLib"
		
		includedirs "../vendor/libsodium/src/libsodium/include/sodium/"
		includedirs "../vendor/libsodium/builds/msvc/"
		
		-- to not conflict with utils.h
		removeincludedirs "client/shared/"
		
		defines {
			"SODIUM_STATIC",
			"NATIVE_LITTLE_ENDIAN"
		}
		
		files_project "../vendor/libsodium/" {
			"src/**.c",
			"src/libsodium/include/**.h"
		}
	end
}
		