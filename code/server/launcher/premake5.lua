	project "DuplicityMain"
		language "C++"
		kind "ConsoleApp"

		links { "Shared", "CitiCore" }

		add_dependencies { 'vendor:fmtlib', 'vendor:breakpad' }
		
		if os.istarget('windows') then
			links { "psapi", "wininet", "winhttp" }
		else
			links { 'dl', 'pthread' }
		end

		includedirs
		{
			"../../client/citicore/",
			"include/"
		}

		files
		{
			"**.cpp", "**.h", "../../client/common/Error.cpp"
		}

		pchsource "src/StdInc.cpp"
		pchheader "StdInc.h"

		targetname "FXServer"
