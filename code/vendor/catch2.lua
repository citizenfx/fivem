return {
	include = function()
		includedirs { "vendor/catch2/include/", "vendor/catch2/" }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		includedirs { "vendor/catch2/src/", "vendor/catch2/include/" }

		files {
			"vendor/catch2/src/**.cpp"
		}
	end
}
