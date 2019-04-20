-- tinyxml2 with static runtime, as used for launcher
return {
	include = function()
		if not os.istarget('windows') then
			defines { "BOTAN_DLL=" }
		else
			defines { "BOTAN_DLL=__declspec(dllimport)" }
		end

		includedirs "vendor/botan/include/"
	end,

	run = function()
		language "C++"
		kind "SharedLib"

		if os.istarget('windows') then
			defines { "BOTAN_DLL=__declspec(dllexport)", "_DISABLE_EXTENDED_ALIGNED_STORAGE" }

			buildoptions '/bigobj'
			
			if _OPTIONS['with-asan'] then
				buildoptions '-mrdrnd -mrdseed'
			end
		else
			defines { "BOTAN_DLL=" }

			buildoptions { '-msse', '-msse2' }
		end

		if os.istarget('windows') then
			links 'ws2_32'

			files {
				"vendor/botan/src/*.cpp",
				"vendor/botan/src/*.h"
			}
		elseif os.istarget('linux') then
			prebuildcommands {
				('cp %s %s'):format(
					path.getabsolute('vendor/botan/include/build_linux.h'),
					path.getabsolute('vendor/botan/include/botan/build.h')
				)
			}

			files {
				"vendor/botan/src/linux/*.cpp",
				"vendor/botan/src/linux/*.h"
			}
		end
	end
}
