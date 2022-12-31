#include <StdInc.h>

#if _WIN32
#include <dbghelp.h>
#include <client/windows/handler/exception_handler.h>
#include <client/windows/crash_generation/client_info.h>
#include <client/windows/crash_generation/crash_generation_client.h>
#include <client/windows/crash_generation/crash_generation_server.h>
#include <common/windows/http_upload.h>

#include <cfx_version.h>

static google_breakpad::ExceptionHandler* g_exceptionHandler;

using namespace google_breakpad;

namespace google_breakpad
{
class AutoExceptionHandler
{
public:
	static LONG HandleException(EXCEPTION_POINTERS* exinfo)
	{
		return ExceptionHandler::HandleException(exinfo);
	}
};
}

void InitializeMiniDumpOverride()
{
	auto CoreSetExceptionOverride = (void (*)(LONG(*)(EXCEPTION_POINTERS*)))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreSetExceptionOverride");

	if (CoreSetExceptionOverride)
	{
		CoreSetExceptionOverride(AutoExceptionHandler::HandleException);
	}
}

static DWORD BeforeTerminateHandler(LPVOID arg)
{
	__try
	{
		auto coreRt = GetModuleHandleW(L"CoreRT.dll");

		if (coreRt)
		{
			auto func = (void(*)(void*))GetProcAddress(coreRt, "CoreOnProcessAbnormalTermination");

			if (func)
			{
				func(arg);
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return 0;
}

void InitializeDumpServer(int inheritedHandle, int parentPid)
{
	using namespace google_breakpad;

	static bool g_running = true;

	HANDLE inheritedHandleBit = (HANDLE)inheritedHandle;
	static std::wstring crashDirectory = MakeRelativeCitPath(L"crashes");

	CrashGenerationServer::OnClientConnectedCallback connectCallback = [](void*, const ClientInfo* info)
	{

	};

	CrashGenerationServer::OnClientDumpRequestCallback dumpCallback = [](void*, const ClientInfo* client, const std::wstring*)
	{
		// We have to get the address of EXCEPTION_INFORMATION from
		// the client process address space.
		EXCEPTION_POINTERS* client_ex_info = NULL;
		if (!client->GetClientExceptionInfo(&client_ex_info)) {
			return;
		}

		std::wstring dumpPath;

		DWORD client_thread_id = 0;
		if (!client->GetClientThreadId(&client_thread_id)) {
			return;
		}

		MDRawAssertionInfo client_assert_info = { {0} };

		std::wstring full_dump_path;

		SIZE_T bytes_read = 0;
		if (!ReadProcessMemory(client->process_handle(),
			client->assert_info(),
			&client_assert_info,
			sizeof(client_assert_info),
			&bytes_read))
		{
			return;
		}

		{
			MinidumpGenerator dump_generator(crashDirectory,
				client->process_handle(),
				client->pid(),
				client_thread_id,
				GetCurrentThreadId(),
				client_ex_info,
				client_assert_info.type ? client->assert_info() : nullptr,
				client->dump_type(),
				true);

			if (!dump_generator.GenerateDumpFile(&dumpPath))
			{
				return;
			}

			if (client->dump_type() & MiniDumpWithFullMemory)
			{
				if (!dump_generator.GenerateFullDumpFile(&full_dump_path))
				{
					return;
				}
			}

			if (!dump_generator.WriteMinidump())
			{
				return;
			}
		}

		std::map<std::wstring, std::wstring> parameters;
		parameters[L"sentry[release]"] = ToWide(GIT_TAG);

		std::wstring responseBody;
		int responseCode;

		std::map<std::wstring, std::wstring> files;
		files[L"upload_file_minidump"] = dumpPath;

		{
			std::string friendlyReason = "Server crashed.";

			// try to send the clients a bit of a obituary notice
			LPVOID memPtr = VirtualAllocEx(client->process_handle(), NULL, friendlyReason.size() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			if (memPtr)
			{
				WriteProcessMemory(client->process_handle(), memPtr, friendlyReason.data(), friendlyReason.size() + 1, NULL);
			}

			// we assume that (since unlike the game, we enable ASLR), as both processes are usually created around the same time, that we'll be the same binary
			// and have the same ASLR location. this might need some better logic in the future if this turns out to not be the case.
			//
			// however, if it isn't - the target will just crash again.
			HANDLE hThread = CreateRemoteThread(client->process_handle(), NULL, 0, BeforeTerminateHandler, memPtr, 0, NULL);

			if (hThread)
			{
				WaitForSingleObject(hThread, 15000);
				CloseHandle(hThread);
			}
		}

		DebugActiveProcess(GetProcessId(client->process_handle()));

		printf("\n\n=================================================================\n\x1b[31mFXServer crashed.\x1b[0m\nA dump can be found at %s.\n", ToNarrow(dumpPath).c_str());

		if (!full_dump_path.empty())
		{
			printf("In addition to this, a full dump file has been generated at %s.\n\n", ToNarrow(full_dump_path).c_str());
		}

		bool uploadCrashes = true;

		if (uploadCrashes && HTTPUpload::SendMultipartPostRequest(L"https://sentry.fivem.net/api/3/minidump/?sentry_key=15f0687e23d74681b5c1f80a0f3a00ed", parameters, files, nullptr, &responseBody, &responseCode))
		{
			printf("Crash report ID: %s\n", ToNarrow(responseBody).c_str());
		}
		else
		{
			printf("Failed to automatically report the crash. Please submit the crash dump to the project developers.\n");
		}

		printf("=================================================================\n");

		DebugActiveProcessStop(GetProcessId(client->process_handle()));

		TerminateProcess(client->process_handle(), -2);
	};

	CrashGenerationServer::OnClientExitedCallback exitCallback = [](void*, const ClientInfo* info)
	{
	};

	CrashGenerationServer::OnClientUploadRequestCallback uploadCallback = [](void*, DWORD)
	{

	};

	std::wstring pipeName = L"\\\\.\\pipe\\CitizenFX_Server_Dump";

	static HANDLE parentProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, parentPid);

	CrashGenerationServer server(pipeName, nullptr, connectCallback, nullptr, dumpCallback, nullptr, exitCallback, nullptr, uploadCallback, nullptr, false, &crashDirectory);
	if (server.Start())
	{
		SetEvent(inheritedHandleBit);
		WaitForSingleObject(parentProcess, INFINITE);
	}
}


bool InitializeExceptionHandler(int argc, char* argv[])
{
	using namespace google_breakpad;

	// don't initialize when under a debugger, as debugger filtering is only done when execution gets to UnhandledExceptionFilter in basedll
	if (IsDebuggerPresent())
	{
		return false;
	}

	std::wstring crashDirectory = MakeRelativeCitPath(_P("crashes"));
	CreateDirectory(crashDirectory.c_str(), nullptr);

	wchar_t* dumpServerBit = wcsstr(GetCommandLine(), L"-dumpserver");

	if (dumpServerBit)
	{
		wchar_t* parentPidBit = wcsstr(GetCommandLine(), L"-parentpid:");

		InitializeDumpServer(wcstol(&dumpServerBit[12], nullptr, 10), wcstol(&parentPidBit[11], nullptr, 10));

		return true;
	}

	CrashGenerationClient* client = new CrashGenerationClient(L"\\\\.\\pipe\\CitizenFX_Server_Dump", (MINIDUMP_TYPE)(MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo | MiniDumpWithFullMemory), new CustomClientInfo());

	if (!client->Register())
	{
		auto applicationHandle = GetModuleHandle(nullptr);
		wchar_t applicationName[512];

		GetModuleFileNameW(applicationHandle, applicationName, _countof(applicationName));

		// prepare initial structures
		STARTUPINFO startupInfo = { 0 };
		startupInfo.cb = sizeof(STARTUPINFO);

		PROCESS_INFORMATION processInfo = { 0 };

		// create an init handle
		SECURITY_ATTRIBUTES securityAttributes = { 0 };
		securityAttributes.bInheritHandle = TRUE;

		HANDLE initEvent = CreateEvent(&securityAttributes, TRUE, FALSE, nullptr);

		// create the command line including argument
		wchar_t commandLine[MAX_PATH * 8];
		if (_snwprintf(commandLine, _countof(commandLine), L"\"%s\" -dumpserver:%i -parentpid:%i", applicationName, (int)initEvent, GetCurrentProcessId()) >= _countof(commandLine))
		{
			return false;
		}

		BOOL result = CreateProcess(applicationName, commandLine, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startupInfo, &processInfo);

		if (result)
		{
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}

		DWORD waitResult = WaitForSingleObject(initEvent, 7500);
		if (!client->Register())
		{
			printf("Could not register with breakpad server.\n");
		}
	}

	g_exceptionHandler = new ExceptionHandler(
		L"",
		[](void* context, EXCEPTION_POINTERS* exinfo,
			MDRawAssertionInfo* assertion)
	{
		return true;
	},
		[](const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
	{
		return succeeded;
	},
		nullptr,
		ExceptionHandler::HANDLER_ALL,
		client
		);

	g_exceptionHandler->set_handle_debug_exceptions(true);

	return false;
}
#else
#define HANDLE_EINTR(x) ([&]() -> decltype(x) { \
  decltype(x) eintr_wrapper_result; \
  do { \
    eintr_wrapper_result = (x); \
  } while (eintr_wrapper_result == -1 && errno == EINTR); \
  return eintr_wrapper_result; \
})()


#include <spawn.h>
#include <sys/prctl.h>

#include <client/linux/handler/exception_handler.h>
#include <client/linux/crash_generation/client_info.h>
#include <client/linux/crash_generation/crash_generation_client.h>
#include <client/linux/crash_generation/crash_generation_server.h>
#include <common/linux/http_upload.h>

#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <cfx_version.h>

static google_breakpad::ExceptionHandler* g_exceptionHandler;

void InitializeDumpServer(int serverFd, int pipeFd)
{
	using namespace google_breakpad;

	static bool g_running = true;

	static std::string crashDirectory = MakeRelativeCitPath("crashes");

	CrashGenerationServer::OnClientDumpRequestCallback dumpCallback = [](void*, const ClientInfo* client, const std::string* dumpPath)
	{
		std::map<std::string, std::string> parameters;
		parameters["sentry[release]"] = GIT_TAG;

		std::string responseBody;
		long responseCode;

		std::map<std::string, std::string> files;
		files["upload_file_minidump"] = *dumpPath;

		printf("\n\n=================================================================\n\x1b[31mFXServer crashed.\x1b[0m\nA dump can be found at %s.\n", dumpPath->c_str());

		bool uploadCrashes = true;
		std::string ed;

		if (uploadCrashes && HTTPUpload::SendRequest("https://sentry.fivem.net/api/3/minidump/?sentry_key=15f0687e23d74681b5c1f80a0f3a00ed", parameters, files, "", "", "", &responseBody, &responseCode, &ed))
		{
			printf("Crash report ID: %s\n", responseBody.c_str());
		}
		else
		{
			printf("Failed to automatically report the crash. Please submit the crash dump to the project developers.\n");
		}

		printf("=================================================================\n");
	};

	CrashGenerationServer server(serverFd, dumpCallback, nullptr, nullptr, nullptr, true, &crashDirectory);
	if (server.Start())
	{
		uint8_t b = 1;
    	write(pipeFd, &b, sizeof(b));
		
		while (true)
		{
			sleep(5);
		}
	}
}

#include <elf.h>
#include <link.h>
#include <sys/auxv.h>

extern "C" int __ehdr_start;

bool InitializeExceptionHandler(int argc, char* argv[])
{
	using namespace google_breakpad;

	std::string crashDirectory = MakeRelativeCitPath(_P("crashes"));
	mkdir(crashDirectory.c_str(), 0777);

	char* dumpServerBit = nullptr;
	char* parentPidBit = nullptr;

	for (int i = 0; i < argc; i++)
	{
		if (strstr(argv[i], "-dumpserver"))
		{
			dumpServerBit = strstr(argv[i], "-dumpserver");
		}
		else if (strstr(argv[i], "-parentppe:"))
		{
			parentPidBit = strstr(argv[i], "-parentppe:");
		}
	}

	if (dumpServerBit)
	{
		prctl(PR_SET_PDEATHSIG, SIGKILL);

		InitializeDumpServer(strtol(&dumpServerBit[12], nullptr, 10), strtol(&parentPidBit[11], nullptr, 10));

		return true;
	}

	// start a dump server no matter what. POSIX is weird, so no named stuff
	int server_fd, client_fd;

	if (!CrashGenerationServer::CreateReportChannel(&server_fd, &client_fd))
	{
		return false;
	}

	int fds[2];
	if (pipe(fds) == -1)
	{
		return false;
	}

	std::vector<char*> args;

	if (access("/lib/ld-musl-x86_64.so.1", F_OK) != -1)
	{
		args.resize(3);
		args[0] = strdup(MakeRelativeCitPath(L"FXServer").c_str());
		args[1] = strdup(va("-dumpserver:%d", server_fd));
		args[2] = strdup(va("-parentppe:%d", fds[1]));
		args.push_back(nullptr);
	}
	else
	{
		args.push_back(strdup(MakeRelativeCitPath("../../../run.sh").c_str()));
		args.push_back(strdup(va("-dumpserver:%d", server_fd)));
		args.push_back(strdup(va("-parentppe:%d", fds[1])));
		args.push_back(nullptr);
	}

	// move our DT_DEBUG on top of the ldso's DT_DEBUG
	ElfW(Phdr)* phdr = reinterpret_cast<ElfW(Phdr)*>(getauxval(AT_PHDR));
	char* base;
	int phnum = getauxval(AT_PHNUM);
	if (phnum && phdr)
	{
		// Assume the program base is at the beginning of the same page as the PHDR
		base = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(phdr) & ~0xfff);
		auto base_src = (char*)&__ehdr_start;

		// Search for the program PT_DYNAMIC segment
		ElfW(Addr) dyn_addr = 0;
		for (; phnum >= 0; phnum--, phdr++)
		{
			// Adjust base address with the virtual address of the PT_LOAD segment
			// corresponding to offset 0
			if (phdr->p_type == PT_LOAD && phdr->p_offset == 0)
				base -= phdr->p_vaddr;
			if (phdr->p_type == PT_DYNAMIC)
				dyn_addr = phdr->p_vaddr;
		}

		ElfW(Ehdr)* ehdr = (ElfW(Ehdr)*)base_src;

		phdr = (ElfW(Phdr)*)(base_src + ehdr->e_phoff);
		ElfW(Addr) dyn_addr_src = 0;
		int e_phnum = ehdr->e_phnum;

		for (; e_phnum >= 0; e_phnum--, phdr++)
		{
			// Adjust base address with the virtual address of the PT_LOAD segment
			// corresponding to offset 0
			if (phdr->p_type == PT_DYNAMIC)
				dyn_addr_src = phdr->p_vaddr;
		}

		if (dyn_addr && dyn_addr_src)
		{
			ElfW(Dyn)* dynamic = reinterpret_cast<ElfW(Dyn)*>(dyn_addr + base);
			ElfW(Dyn)* dynamic_src = reinterpret_cast<ElfW(Dyn)*>(dyn_addr_src + base_src);

			ElfW(Dyn)* tgtDyn = NULL;

			for (int i = 0;; i++)
			{
				if (dynamic[i].d_tag == DT_BIND_NOW)
				{
					tgtDyn = &dynamic[i];
					continue;
				}
				else if (dynamic[i].d_tag == DT_NULL)
				{
					break;
				}
			}

			for (int i = 0;; i++)
			{
				if (dynamic_src[i].d_tag == DT_DEBUG)
				{
					if (tgtDyn)
					{
						mprotect((void*)tgtDyn, sizeof(void*) * 2, PROT_READ | PROT_WRITE);
						*tgtDyn = dynamic_src[i];
					}

					continue;
				}
				else if (dynamic_src[i].d_tag == DT_NULL)
				{
					break;
				}
			}
		}
	}

	posix_spawn(nullptr, args[0], nullptr, nullptr, args.data(), nullptr);

	// wait for server init
	struct pollfd pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.fd = fds[0];
	pfd.events = POLLIN | POLLERR;
	int r = HANDLE_EINTR(poll(&pfd, 1, 5000));
	if (r != 1 || (pfd.revents & POLLIN) != POLLIN)
	{
		return false;
	}

	uint8_t b;
	read(fds[0], &b, sizeof(b));
	close(fds[0]);

	g_exceptionHandler = new ExceptionHandler(
		MinidumpDescriptor(crashDirectory.c_str()),
		nullptr,
		nullptr,
		nullptr,
		true,
		client_fd
		);

	return false;
}

#endif
