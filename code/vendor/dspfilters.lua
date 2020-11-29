return {
	include = function()
		includedirs { "../vendor/dspfilters/shared/DSPFilters/include/" }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		files_project "../vendor/dspfilters/shared/DSPFilters/" {
			"include/**.h",
			"source/**.cpp",
		}
	end
}