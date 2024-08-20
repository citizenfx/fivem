return {
	include = function()
		includedirs {
			"../vendor/eastl/include/",
			"../vendor/eabase/include/Common/",
		}
		
		defines { "EASTL_OPENSOURCE=1","EA_DEPRECATIONS_FOR_2024_APRIL=EA_DISABLED", "EA_DEPRECATIONS_FOR_2024_SEPT=EA_DISABLED", "EA_DEPRECATIONS_FOR_2025_APRIL=EA_DISABLED", "EASTL_USER_DEFINED_ALLOCATOR=1" }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		files_project "../vendor/eastl/" {
			"include/EASTL/*.h",
			"include/EASTL/bonus/*.h",
			"source/*.cpp"
		}

		files_project "../vendor/eabase" {
			"include/Common/EABase/**.h"
		}
	end
}