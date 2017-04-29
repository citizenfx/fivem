project "CitiCore"
	targetname "CoreRT" 
	language "C++"
	kind "SharedLib"

	includedirs "."

	files
	{
		"**.cpp",
		"**.h",
		"../common/Error.cpp",
		"../common/StdInc.cpp"
	}

	files
	{
		"**.idl"
	}

	add_dependencies { 'vendor:boost_program_options' }
	add_dependencies { 'vendor:minhook', 'vendor:udis86' }

	links { "Shared" }

	defines "COMPILING_CORE"

	pchsource "../common/StdInc.cpp"
	pchheader "StdInc.h"

	configuration "not windows"
		links { "dl", "c++" }
