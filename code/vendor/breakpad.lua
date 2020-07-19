return {
	include = function()
		includedirs "../vendor/breakpad/src/"

		configuration "linux"
			includedirs { "vendor/breakpad/src/" }

		configuration {}
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		if _OPTIONS['game'] ~= 'server' then
			staticruntime 'On'
		end

		configuration "linux"
			includedirs { "vendor/breakpad/src/" }

			files_project "../vendor/breakpad/src/" {
				"client/linux/handler/exception_handler.cc",
				"client/linux/handler/minidump_descriptor.cc",
				"client/linux/dump_writer_common/thread_info.cc",
				"client/linux/dump_writer_common/ucontext_reader.cc",
				"client/linux/crash_generation/crash_generation_client.cc",
				"client/linux/crash_generation/crash_generation_server.cc",
				"client/linux/microdump_writer/microdump_writer.cc",
				"client/linux/minidump_writer/linux_core_dumper.cc",
				"client/linux/minidump_writer/linux_dumper.cc",
				"client/linux/minidump_writer/linux_ptrace_dumper.cc",
				"client/linux/minidump_writer/minidump_writer.cc",
				"client/minidump_file_writer.cc",
				"client/linux/log/log.cc",
				"common/md5.cc",
				"common/string_conversion.cc",
				"common/linux/elf_core_dump.cc",
				"common/linux/elfutils.cc",
				"common/linux/file_id.cc",
				"common/linux/guid_creator.cc",
				"common/linux/linux_libc_support.cc",
				"common/linux/memory_mapped_file.cc",
				"common/linux/safe_readlink.cc",
				"common/linux/http_upload.cc",	
				"common/convert_UTF.cc",
				"common/linux/breakpad_getcontext.S",
			}

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
