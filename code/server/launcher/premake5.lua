	project "DuplicityMain"
		language "C++"
		kind "ConsoleApp"

		links { "Shared", "CitiCore" }

		add_dependencies { 'vendor:fmtlib', 'vendor:breakpad' }
		
		if os.istarget('windows') then
			links { "psapi", "wininet", "winhttp" }
			flags { "NoManifest", "NoImportLib" }
			files { "server.rc" }

			-- match the 4 MB stack size set on Linux in Main.cpp
			-- again: 1.5 MB is required for V8
			linkoptions '/STACK:0x400000'
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
