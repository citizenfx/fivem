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