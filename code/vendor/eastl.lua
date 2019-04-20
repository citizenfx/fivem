return {
	include = function()
		includedirs {
			"../vendor/eastl/include/",
			"../vendor/eastl/test/packages/EABase/include/Common/",
		}
		
		defines { "EASTL_OPENSOURCE=1" }
	end,
	
	run = function()
		language "C++"
		kind "StaticLib"
		
		files_project "../vendor/eastl/" {
			"include/EASTL/*.h",
			"include/EASTL/bonus/*.h",
			"source/*.cpp",
			"test/packages/EABase/include/Common/EABase/**.h"
		}
	end
}