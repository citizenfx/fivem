/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <time.h>
#include <dbghelp.h>
#include <client/windows/handler/exception_handler.h>
#include <client/windows/crash_generation/client_info.h>
#include <client/windows/crash_generation/crash_generation_server.h>
#include <common/windows/http_upload.h>

#include <commctrl.h>
#include <shellapi.h>

using namespace google_breakpad;

static ExceptionHandler* g_exceptionHandler;

void InitializeDumpServer(int inheritedHandle, int parentPid)
{
	static bool g_running = true;

	HANDLE inheritedHandleBit = (HANDLE)inheritedHandle;
	HANDLE parentProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, parentPid);

	CrashGenerationServer::OnClientConnectedCallback connectCallback = [] (void*, const ClientInfo* info)
	{

	};

	CrashGenerationServer::OnClientDumpRequestCallback dumpCallback = [] (void*, const ClientInfo* info, const std::wstring* filePath)
	{
		std::map<std::wstring, std::wstring> parameters;
#ifdef GTA_NY
		parameters[L"ProductName"] = L"CitizenFX";
		parameters[L"Version"] = L"1.0";
		parameters[L"BuildID"] = L"20141213000000"; // todo i bet
#elif defined(GTA_FIVE)
		parameters[L"ProductName"] = L"FiveM";
		parameters[L"Version"] = L"1.0";
		parameters[L"BuildID"] = L"20151104"; // todo i bet
#endif

		parameters[L"ReleaseChannel"] = L"release";

		std::wstring responseBody;
		int responseCode;

		std::map<std::wstring, std::wstring> files;
		files[L"upload_file_minidump"] = *filePath;

#ifdef GTA_NY
		if (HTTPUpload::SendRequest(L"http://cr.citizen.re:5100/submit", parameters, files, nullptr, &responseBody, &responseCode))
#elif defined(GTA_FIVE)
		if (HTTPUpload::SendRequest(L"http://report-crash.fivem.net/submit", parameters, files, nullptr, &responseBody, &responseCode))
#endif
		{
			TASKDIALOGCONFIG taskDialogConfig = { 0 };
			taskDialogConfig.cbSize = sizeof(taskDialogConfig);
			taskDialogConfig.hInstance = GetModuleHandle(nullptr);
			taskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_EXPAND_FOOTER_AREA;
			taskDialogConfig.dwCommonButtons = TDCBF_CLOSE_BUTTON;
			taskDialogConfig.pszWindowTitle = PRODUCT_NAME L" Fatal Error";
			taskDialogConfig.pszMainIcon = TD_ERROR_ICON;
			taskDialogConfig.pszMainInstruction = PRODUCT_NAME L" has stopped working";
			taskDialogConfig.pszContent = L"An error caused " PRODUCT_NAME L" to stop working. A crash report has been uploaded to the " PRODUCT_NAME L" developers. If you require immediate support, please visit <A HREF=\"http://forum.citizen.re/\">Citizen.re</A> and mention the details below.";
			taskDialogConfig.pszExpandedInformation = va(L"Crash ID: %s(use Ctrl+C to copy)", responseBody.substr(8).c_str());
			taskDialogConfig.pfCallback = [] (HWND, UINT type, WPARAM wParam, LPARAM lParam, LONG_PTR data)
			{
				if (type == TDN_HYPERLINK_CLICKED)
				{
					ShellExecute(nullptr, L"open", (LPCWSTR)lParam, nullptr, nullptr, SW_NORMAL);
				}
				else if (type == TDN_BUTTON_CLICKED)
				{
					return S_OK;
				}

				return S_FALSE;
			};

			TaskDialogIndirect(&taskDialogConfig, nullptr, nullptr, nullptr);
		}
	};

	CrashGenerationServer::OnClientExitedCallback exitCallback = [] (void*, const ClientInfo* info)
	{
	};

	CrashGenerationServer::OnClientUploadRequestCallback uploadCallback = [] (void*, DWORD)
	{

	};

	std::wstring crashDirectory = MakeRelativeCitPath(L"crashes");

	std::wstring pipeName = L"\\\\.\\pipe\\CitizenFX_Dump";

	CrashGenerationServer server(pipeName, nullptr, connectCallback, nullptr, dumpCallback, nullptr, exitCallback, nullptr, uploadCallback, nullptr, true, &crashDirectory);
	if (server.Start())
	{
		SetEvent(inheritedHandleBit);
		WaitForSingleObject(parentProcess, INFINITE);
	}
}

bool InitializeExceptionHandler()
{
	// don't initialize when under a debugger, as debugger filtering is only done when execution gets to UnhandledExceptionFilter in basedll
	if (IsDebuggerPresent())
	{
		return false;
	}

	std::wstring crashDirectory = MakeRelativeCitPath(L"crashes");
	CreateDirectory(crashDirectory.c_str(), nullptr);

	wchar_t* dumpServerBit = wcsstr(GetCommandLine(), L"-dumpserver");

	if (dumpServerBit)
	{
		wchar_t* parentPidBit = wcsstr(GetCommandLine(), L"-parentpid:");

		InitializeDumpServer(wcstol(&dumpServerBit[12], nullptr, 10), wcstol(&parentPidBit[11], nullptr, 10));

		return true;
	}

	CrashGenerationClient* client = new CrashGenerationClient(L"\\\\.\\pipe\\CitizenFX_Dump", MiniDumpNormal, new CustomClientInfo());

	if (!client->Register())
	{
		wchar_t applicationName[MAX_PATH];
		GetModuleFileName(nullptr, applicationName, _countof(applicationName));

		// prepare initial structures
		STARTUPINFO startupInfo = { 0 };
		startupInfo.cb = sizeof(STARTUPINFO);

		PROCESS_INFORMATION processInfo = { 0 };

		// create an init handle
		SECURITY_ATTRIBUTES securityAttributes = { 0 };
		securityAttributes.bInheritHandle = TRUE;

		HANDLE initEvent = CreateEvent(&securityAttributes, TRUE, FALSE, nullptr);

		// create the command line including argument
		wchar_t commandLine[MAX_PATH * 2];
		_snwprintf(commandLine, sizeof(commandLine), L"%s -dumpserver:%i -parentpid:%i", GetCommandLine(), (int)initEvent, GetCurrentProcessId());

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
							nullptr,
							[] (const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
							{
								return succeeded;
							},
							nullptr,
							ExceptionHandler::HANDLER_ALL,
							client
						);

	g_exceptionHandler->set_handle_debug_exceptions(true);

	// disable Windows' SetUnhandledExceptionFilter
#ifdef _M_AMD64
	DWORD oldProtect;

	LPVOID unhandledFilters[] = { 
		GetProcAddress(GetModuleHandle(L"kernelbase.dll"), "SetUnhandledExceptionFilter"),
		GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetUnhandledExceptionFilter"),
	};

	for (auto& unhandledFilter : unhandledFilters)
	{
		if (unhandledFilter)
		{
			VirtualProtect(unhandledFilter, 4, PAGE_EXECUTE_READWRITE, &oldProtect);

			*(uint8_t*)unhandledFilter = 0xC3;
		}
	}
#endif

	return false;
}