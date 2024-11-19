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

		filter { "options:game=ny" }
			targetname "LibertyM"

		filter { "options:game=payne" }
			targetname "CitizenPayne"

		filter { "options:game=five" }
			targetname "VMP"
