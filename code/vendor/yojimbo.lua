return {
	include = function()
		includedirs { "../vendor/yojimbo", "../vendor/netcode.io", "../vendor/reliable.io" }
		
		links { 'mbedtls', 'sodium' }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		add_dependencies { 'vendor:mbedtls', 'vendor:sodium' }
		
		files {
			"../vendor/yojimbo/yojimbo.h",
			"../vendor/yojimbo/yojimbo.cpp",
			"../vendor/yojimbo/tlsf/tlsf.h",
			"../vendor/yojimbo/tlsf/tlsf.c",
			"../vendor/netcode.io/netcode.c",
			"../vendor/netcode.io/netcode.h",
			"../vendor/reliable.io/reliable.c",
			"../vendor/reliable.io/reliable.h"
		}
	end
}