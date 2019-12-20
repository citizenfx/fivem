project "ipfsdl"
	targetname "ipfsdl"
	language "C++"
	kind "SharedLib"

	includedirs "."

	files
	{
		"**.cpp",
		"**.h",
		"**.cc",
		"../common/Error.cpp",
		"../common/StdInc.cpp"
	}

	add_dependencies { 'vendor:grpc', 'vendor:tbb', 'vendor:protobuf_lite' }

	links { "Shared" }

	defines "COMPILING_IPFSDL"
