return {
	include = function()
		includedirs "../vendor/breakpad/src/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		flags { "StaticRuntime" }

		configuration "windows"
			files_project "../vendor/breakpad/src/" {
				"client/windows/handler/exception_handler.cc",
				"client/windows/crash_generation/client_info.cc",
				"client/windows/crash_generation/crash_generation_client.cc",
				"client/windows/crash_generation/crash_generation_server.cc",
				"client/windows/crash_generation/minidump_generator.cc",
				"common/windows/guid_string.cc",
				"common/windows/http_upload.cc",
				"common/windows/string_utils.cc",
			}
	end
}