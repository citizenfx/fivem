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

	add_dependencies { 'vendor:boost_program_options', 'vendor:tbb' }

	if os.istarget('windows') then
		add_dependencies { 'vendor:minhook', 'vendor:udis86' }
	end

	links { "Shared" }

	defines "COMPILING_CORE"

	configuration "not windows"
		links { "dl" }
