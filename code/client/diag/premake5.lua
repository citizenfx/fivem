	project "CfxDiag"
		language "C++"
		kind "ConsoleApp"
		
		defines { "COMPILING_LAUNCH", "COMPILING_DIAG" }

		symbols "Full"
		
		links { "SharedLibc" }
		
		files
		{
			"**.cpp", "**.h", 
			"diagutil.rc",
			"../common/Error.cpp"
		}
		
		pchsource "StdInc.cpp"
		pchheader "StdInc.h"

		add_dependencies { 'vendor:curl-crt', 'vendor:cpr-crt' }
		
		staticruntime 'On'

		filter { "options:game=five" }
			targetname "VMP_Diag"
