-- tinyxml2 with static runtime, as used for launcher
return {
	include = function()
		if not os.is('windows') then
			defines { "BOTAN_DLL=" }
		else
			defines { "BOTAN_DLL=__declspec(dllimport)" }
		end

		includedirs "vendor/botan/include/"
	end,

	run = function()
		language "C++"
		kind "SharedLib"

		if os.is('windows') then
			defines { "BOTAN_DLL=__declspec(dllexport)" }

			buildoptions '/bigobj'
		else
			defines { "BOTAN_DLL=" }

			buildoptions { '-msse', '-msse2' }
		end

		if os.is('windows') then
			files {
				"vendor/botan/src/*.cpp",
				"vendor/botan/src/*.h"
			}
		elseif os.is('linux') then
			files {
				"vendor/botan/src/linux/*.cpp",
				"vendor/botan/src/linux/*.h"
			}
		end
	end
}
