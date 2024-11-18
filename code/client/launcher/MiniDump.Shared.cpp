#include "StdInc.h"
#include <client/windows/handler/exception_handler.h>
#include <client/windows/crash_generation/crash_generation_client.h>

#include <CfxState.h>
#include <CfxSubProcess.h>
#include <HostSharedData.h>

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

static ExceptionHandler* g_exceptionHandler;

extern "C" IMAGE_DOS_HEADER __ImageBase;
extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
#ifdef _M_AMD64
extern "C" void WINAPI __security_init_cookie();
#endif

static bool initialized = false;

extern "C" DLL_EXPORT void EarlyInitializeExceptionHandler()
{
	if (initialized)
	{
		return;
	}

#ifdef _M_AMD64
	__security_init_cookie();
#endif
	_CRT_INIT((HINSTANCE)&__ImageBase, DLL_PROCESS_ATTACH, nullptr);
	_CRT_INIT((HINSTANCE)&__ImageBase, DLL_THREAD_ATTACH, nullptr);
}

extern "C" DLL_EXPORT bool TerminateForException(PEXCEPTION_POINTERS exception)
{
	if (g_exceptionHandler)
	{
		return g_exceptionHandler->WriteMinidumpForException(exception);
	}

	return false;
}

#if defined(LAUNCHER_PERSONALITY_MAIN)
extern void InitializeDumpServer(int inheritedHandle, int parentPid);
#endif

bool InitializeExceptionServer()
{
	wchar_t* dumpServerBit = wcsstr(GetCommandLine(), L"-dumpserver");

	if (dumpServerBit)
	{
		wchar_t* parentPidBit = wcsstr(GetCommandLine(), L"-parentpid:");

#if defined(LAUNCHER_PERSONALITY_MAIN)
		InitializeDumpServer(wcstol(&dumpServerBit[12], nullptr, 10), wcstol(&parentPidBit[11], nullptr, 10));
#endif

		return true;
	}

	return false;
}


// a safe exception buffer to be allocated in low (32-bit) memory to contain what() data
struct ExceptionBuffer
{
	char data[4096];
};

static ExceptionBuffer* g_exceptionBuffer;

static void AllocateExceptionBuffer()
{
	auto _NtAllocateVirtualMemory = (HRESULT(WINAPI*)(
	HANDLE ProcessHandle,
	PVOID * BaseAddress,
	ULONG_PTR ZeroBits,
	PSIZE_T RegionSize,
	ULONG AllocationType,
	ULONG Protect)) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtAllocateVirtualMemory");

	PVOID baseAddr = NULL;
	SIZE_T size = sizeof(ExceptionBuffer);

	if (SUCCEEDED(_NtAllocateVirtualMemory(GetCurrentProcess(), &baseAddr, 0xFFFFFFFF80000000, &size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)))
	{
		g_exceptionBuffer = (ExceptionBuffer*)baseAddr;
	}
}

extern "C" DLL_EXPORT DWORD WINAPI RemoteExceptionFunc(LPVOID objectPtr)
{
	__try
	{
		std::exception* object = (std::exception*)objectPtr;

		if (g_exceptionBuffer)
		{
			strncpy(g_exceptionBuffer->data, object->what(), sizeof(g_exceptionBuffer->data));
			g_exceptionBuffer->data[sizeof(g_exceptionBuffer->data) - 1] = '\0';

			return (DWORD)(DWORD_PTR)g_exceptionBuffer;
		}

		return 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}
}

extern "C" DLL_EXPORT DWORD WINAPI BeforeTerminateHandler(LPVOID arg)
{
	__try
	{
		auto coreRt = GetModuleHandleW(L"CoreRT.dll");

		if (coreRt)
		{
			auto func = (void (*)(void*))GetProcAddress(coreRt, "CoreOnProcessAbnormalTermination");

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

extern "C" DLL_EXPORT DWORD WINAPI TryCollectCrashLog(LPVOID arg)
{
	auto argString = (const char*)arg;

	if (argString)
	{
		__try
		{
			auto coreRt = GetModuleHandleW(L"CoreRT.dll");

			if (coreRt)
			{
				auto func = (bool (*)(const char*))GetProcAddress(coreRt, "CoreCollectCrashLog");

				if (func)
				{
					func(argString);
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	return 0;
}

static bool connected = false;

bool MinidumpInitialized()
{
	return connected;
}

extern "C" DLL_EXPORT bool InitializeExceptionHandler()
{
	if (initialized)
	{
		return false;
	}

	initialized = true;

	if (InitializeExceptionServer())
	{
		return true;
	}

	AllocateExceptionBuffer();

	// don't initialize when under a debugger, as debugger filtering is only done when execution gets to UnhandledExceptionFilter in basedll
	bool isDebugged = false;

	if (IsDebuggerPresent())
	{
		isDebugged = true;
	}

	std::wstring crashDirectory = MakeRelativeCitPath(L"crashes");
	CreateDirectory(crashDirectory.c_str(), nullptr);

	bool bigMemoryDump = false;

	std::wstring fpath = MakeRelativeCitPath(L"VMP.ini");

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		bigMemoryDump = (GetPrivateProfileInt(L"Game", L"EnableFullMemoryDump", 0, fpath.c_str()) != 0);
	}

	auto mdType = (MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo);

	if (bigMemoryDump)
	{
		mdType |= MiniDumpWithFullMemory;
	}

	CrashGenerationClient* client = new CrashGenerationClient(L"\\\\.\\pipe\\CitizenFX_Dump", (MINIDUMP_TYPE)mdType, new CustomClientInfo());

	if (!client->Register())
	{
		auto applicationName = MakeCfxSubProcess(L"DumpServer");

		// prepare initial structures
		STARTUPINFO startupInfo = { 0 };
		startupInfo.cb = sizeof(STARTUPINFO);

		PROCESS_INFORMATION processInfo = { 0 };

		// create an init handle
		SECURITY_ATTRIBUTES securityAttributes = { 0 };
		securityAttributes.bInheritHandle = TRUE;

		HANDLE initEvent = CreateEvent(&securityAttributes, TRUE, FALSE, nullptr);

		HostSharedData<CfxState> hostData("CfxInitState");

		// create the command line including argument
		wchar_t commandLine[MAX_PATH * 8];
		if (_snwprintf(commandLine, _countof(commandLine), L"\"%s\" -dumpserver:%i -parentpid:%i", applicationName, (int)initEvent, hostData->GetInitialPid()) >= _countof(commandLine))
		{
			return false;
		}

		BOOL result = CreateProcess(applicationName, commandLine, nullptr, nullptr, TRUE, CREATE_BREAKAWAY_FROM_JOB, nullptr, nullptr, &startupInfo, &processInfo);

		if (result)
		{
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}

		DWORD waitResult = WaitForSingleObject(initEvent, 
#ifdef _DEBUG
			1500
#else
			7500
#endif
		);

		if (!isDebugged)
		{
			if (!client->Register())
			{
				trace("Could not register with breakpad server.\n");
			}
		}
	}

	if (isDebugged)
	{
		return false;
	}

	g_exceptionHandler = new ExceptionHandler(
							L"",
							[](void* context, EXCEPTION_POINTERS* exinfo,
								MDRawAssertionInfo* assertion)
							{
								return true;
							},
							[] (const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
							{
								return succeeded;
							},
							nullptr,
							ExceptionHandler::HANDLER_ALL,
							client
						);

	g_exceptionHandler->set_handle_debug_exceptions(true);
	connected = true;

	// disable Windows' SetUnhandledExceptionFilter
	DWORD oldProtect;

	LPVOID unhandledFilters[] = { 
		GetProcAddress(GetModuleHandle(L"kernelbase.dll"), "SetUnhandledExceptionFilter"),
		GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetUnhandledExceptionFilter"),
	};

	for (auto unhandledFilter : unhandledFilters)
	{
		if (unhandledFilter)
		{
			VirtualProtect(unhandledFilter, 4, PAGE_EXECUTE_READWRITE, &oldProtect);

#ifdef _M_AMD64
			*(uint8_t*)unhandledFilter = 0xC3;
#else
			*(uint32_t*)unhandledFilter = 0x900004C2;
#endif

			VirtualProtect(unhandledFilter, 4, oldProtect, &oldProtect);
		}
	}

	return false;
}
