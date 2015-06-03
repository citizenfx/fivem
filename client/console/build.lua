	project "CitiCon"
		language "C++"
		kind "ConsoleApp"
		
		defines "COMPILING_CONSOLE"
		
		links { "SharedLibc" }

		linkoptions "/ENTRY:wmainCRTStartup"

		-- set objdir to prevent conflict with launcher
		objdir ("build/" .. _OPTIONS['game'] .. "/obj/%{cfg.buildcfg}/consoleApp/")

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