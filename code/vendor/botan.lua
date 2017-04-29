-- tinyxml2 with static runtime, as used for launcher
return {
	include = function()
		includedirs "vendor/botan/include/"
	end,

	run = function()
		language "C++"
		kind "SharedLib"

		defines { "BOTAN_DLL=__declspec(dllexport)" }

		buildoptions '/bigobj'

		files {
			"vendor/botan/src/*.cpp",
			"vendor/botan/src/*.h"
		}
	end
}