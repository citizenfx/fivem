return {
	include = function()
		if not os.istarget('windows') then
			defines { "BOTAN_DLL=" }
		else
			defines { "BOTAN_DLL=__declspec(dllimport)" }
		end

		if _OPTIONS['game'] == 'server' then
			includedirs { "vendor/botan_sv/include/", "vendor/botan_sv/include/external/" }
		else
			includedirs "vendor/botan/include/"
		end
	end,

	run = function()
		language "C++"
		kind "SharedLib"
		
		if os.istarget('windows') then
			defines { "BOTAN_DLL=__declspec(dllexport)", "_DISABLE_EXTENDED_ALIGNED_STORAGE" }
			characterset 'MBCS'

			buildoptions '/bigobj'
			
			if _OPTIONS['with-asan'] then
				buildoptions '-mrdrnd -mrdseed'
			end
		else
			defines { "BOTAN_DLL=" }

			buildoptions { '-msse', '-msse2' }
		end

		if os.istarget('windows') then
			links { 'ws2_32', 'crypt32' }

			if _OPTIONS['game'] == 'server' then
				files {
					"vendor/botan_sv/src/*.cpp",
					"vendor/botan_sv/src/*.h"
				}
			else
				files {
					"vendor/botan/src/*.cpp",
					"vendor/botan/src/*.h"
				}				
			end
		elseif os.istarget('linux') then
			prebuildcommands {
				('cp %s %s'):format(
					path.getabsolute('vendor/botan_sv/include/build_linux.h'),
					path.getabsolute('vendor/botan_sv/include/botan/build.h')
				)
			}

			files {
				"vendor/botan_sv/src/linux/*.cpp",
				"vendor/botan_sv/src/linux/*.h"
			}
		end
	end
}
