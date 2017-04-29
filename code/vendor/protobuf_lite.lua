return {
	include = function()
		includedirs { "../vendor/protobuf/src/", "../vendor/protobuf/vsprojects/" }
	end,

	run = function()
		targetname "protobuf_lite"
		language "C++"
		kind "StaticLib"

		configuration "windows"
			buildoptions "/MP /wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305"

		files
		{
			"../vendor/protobuf/src/google/protobuf/io/coded_stream.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/common.cc",
			"../vendor/protobuf/src/google/protobuf/extension_set.cc",
			"../vendor/protobuf/src/google/protobuf/generated_message_util.cc",
			"../vendor/protobuf/src/google/protobuf/message_lite.cc",
			"../vendor/protobuf/src/google/protobuf/arena.cc",
			"../vendor/protobuf/src/google/protobuf/arenastring.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/once.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc",
			"../vendor/protobuf/src/google/protobuf/repeated_field.cc",
			"../vendor/protobuf/src/google/protobuf/wire_format_lite.cc",
			"../vendor/protobuf/src/google/protobuf/io/zero_copy_stream.cc",
			"../vendor/protobuf/src/google/protobuf/io/zero_copy_stream_impl_lite.cc",
			"../vendor/protobuf/src/google/protobuf/stubs/stringprintf.cc"
		}
	end
}