	project "CitiCon"
		language "C++"
		kind "ConsoleApp"
		
		defines "COMPILING_CONSOLE"
		
		links { "SharedLibc" }

		symbolspath "$(TargetDir)CitiCon.pdb"
		linkoptions "/ENTRY:wmainCRTStartup"

		files
		{
			"**.cpp", "**.h", 
			"../common/Error.cpp"
		}
		
		pchsource "StdInc.cpp"
		pchheader "StdInc.h"

		staticruntime "On"

		targetextension '.com'

		configuration "game=ny"
			targetname "CitizenFX"

		configuration "game=payne"
			targetname "CitizenPayne"

		configuration "game=five"
			targetname "FiveM"