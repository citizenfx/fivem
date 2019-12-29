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
	static HANDLE parentProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, parentPid);

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
			LPVOID memPtr = VirtualAllocEx(parentProcess, NULL, friendlyReason.size() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			if (memPtr)
			{
				WriteProcessMemory(parentProcess, memPtr, friendlyReason.data(), friendlyReason.size() + 1, NULL);
			}

			// we assume that (since unlike the game, we enable ASLR), as both processes are usually created around the same time, that we'll be the same binary
			// and have the same ASLR location. this might need some better logic in the future if this turns out to not be the case.
			//
			// however, if it isn't - the target will just crash again.
			HANDLE hThread = CreateRemoteThread(parentProcess, NULL, 0, BeforeTerminateHandler, memPtr, 0, NULL);

			if (hThread)
			{
				WaitForSingleObject(hThread, 15000);
				CloseHandle(hThread);
			}
		}

		DebugActiveProcess(GetProcessId(parentProcess));

		printf("\n\n=================================================================\n\x1b[31mFXServer crashed.\x1b[0m\nA dump can be found at %s.\n", ToNarrow(dumpPath).c_str());

		if (!full_dump_path.empty())
		{
			printf("In addition to this, a full dump file has been generated at %s.\n\n", ToNarrow(full_dump_path).c_str());
		}

		bool uploadCrashes = true;

		if (uploadCrashes && HTTPUpload::SendRequest(L"https://sentry.fivem.net/api/3/minidump/?sentry_key=15f0687e23d74681b5c1f80a0f3a00ed", parameters, files, nullptr, &responseBody, &responseCode))
		{
			printf("Crash report ID: %s\n", ToNarrow(responseBody).c_str());
		}
		else
		{
			printf("Failed to automatically report the crash. Please submit the crash dump to the project developers.\n");
		}

		printf("=================================================================\n");

		DebugActiveProcessStop(GetProcessId(parentProcess));

		TerminateProcess(parentProcess, -2);
	};

	CrashGenerationServer::OnClientExitedCallback exitCallback = [](void*, const ClientInfo* info)
	{
	};

	CrashGenerationServer::OnClientUploadRequestCallback uploadCallback = [](void*, DWORD)
	{

	};

	std::wstring pipeName = L"\\\\.\\pipe\\CitizenFX_Server_Dump";

	CrashGenerationServer server(pipeName, nullptr, connectCallback, nullptr, dumpCallback, nullptr, exitCallback, nullptr, uploadCallback, nullptr, false, &crashDirectory);
	if (server.Start())
	{
		SetEvent(inheritedHandleBit);
		WaitForSingleObject(parentProcess, INFINITE);
	}
}


bool InitializeExceptionHandler()
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
			trace("Could not register with breakpad server.\n");
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
bool InitializeExceptionHandler()
{
	return false;
}
#endif
