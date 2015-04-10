	project "DuplicityMain"
		language "C++"
		kind "ConsoleApp"

		links { "Shared", "CitiCore" }

		includedirs
		{
			"../../client/citicore/",
			"include/"
		}

		files
		{
			"**.cpp", "**.h"
		}

		pchsource "src/StdInc.cpp"
		pchheader "StdInc.h"

		targetname "FXServer"