return {
	include = function()
		includedirs { "vendor/catch2/include/", "vendor/catch2/" }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		defines { "CATCH_AMALGAMATED_CUSTOM_MAIN" }
		
		includedirs { "vendor/catch2/src/", "vendor/catch2/include/" }

		files {
			"vendor/catch2/src/**.cpp"
		}
	end
}
