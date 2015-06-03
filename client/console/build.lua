	project "CitiCon"
		language "C++"
		kind "ConsoleApp"
		
		defines "COMPILING_CONSOLE"
		
		links { "SharedLibc" }

		linkoptions "/ENTRY:wmainCRTStartup /PDB:CitiCon.pdb"

		files
		{
			"**.cpp", "**.h", 
			"../common/Error.cpp"
		}
		
		pchsource "StdInc.cpp"
		pchheader "StdInc.h"

		flags { "StaticRuntime" }

		targetextension '.com'

		configuration "game=ny"
			targetname "CitizenFX"

		configuration "game=payne"
			targetname "CitizenPayne"

		configuration "game=five"
			targetname "FiveM"