if _OPTIONS["game"] ~= "ny" then
	project "peframework"
		language "C++"
		kind "StaticLib"

		includedirs { "peframework/include/" }

		optimize "Speed"

		files {
			"peframework/src/**.cpp",
			"peframework/src/**.h",
			"peframework/include/**.h",
		}

	project "pe_debug"
		language "C++"
		kind "ConsoleApp"
		links { "peframework", "mspdbcorec" }

		optimize "Speed"

		buildoptions { "/Zc:wchar_t-" }

		includedirs { "peframework/include/", "pe_debug/" }
		libdirs { "pe_debug/" }

		files {
			"pe_debug/**.cpp",
			"pe_debug/**.h",
		}

		postbuildcommands {
			("if not exist \"%%{cfg.buildtarget.directory}\\msobj140.dll\" ( copy /y \"%s\" \"%%{cfg.buildtarget.directory}\" )"):format(
				path.getabsolute('../../tools/dbg/bin/msobj140.dll'):gsub('/', '\\')
			),
			("if not exist \"%%{cfg.buildtarget.directory}\\mspdbcore.dll\" ( copy /y \"%s\" \"%%{cfg.buildtarget.directory}\" )"):format(
				path.getabsolute('../../tools/dbg/bin/mspdbcore.dll'):gsub('/', '\\')
			),
		}

	project "retarget_pe"
		language "C++"
		kind "ConsoleApp"
		links { "peframework" }

		optimize "Speed"

		includedirs { "peframework/include/", "retarget_pe/" }

		files {
			"retarget_pe/**.cpp",
		}
end