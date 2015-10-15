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
		parameters[L"ProductName"] = L"CitizenFX";
		parameters[L"Version"] = L"1.0";
		parameters[L"BuildID"] = L"20141213000000"; // todo i bet
		parameters[L"ReleaseChannel"] = L"release";

		std::wstring responseBody;
		int responseCode;

		std::map<std::wstring, std::wstring> files;
		files[L"upload_file_minidump"] = *filePath;

		if (HTTPUpload::SendRequest(L"http://cr.citizen.re:5100/submit", parameters, files, nullptr, &responseBody, &responseCode))
		{
			TASKDIALOGCONFIG taskDialogConfig = { 0 };
			taskDialogConfig.cbSize = sizeof(taskDialogConfig);
			taskDialogConfig.hInstance = GetModuleHandle(nullptr);
			taskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_EXPAND_FOOTER_AREA;
			taskDialogConfig.dwCommonButtons = TDCBF_CLOSE_BUTTON;
			taskDialogConfig.pszWindowTitle = L"CitizenFX Fatal Error";
			taskDialogConfig.pszMainIcon = TD_ERROR_ICON;
			taskDialogConfig.pszMainInstruction = L"CitizenFX has stopped working";
			taskDialogConfig.pszContent = L"An error caused CitizenFX to stop working. A crash report has been uploaded to the CitizenFX developers. If you require immediate support, please visit <A HREF=\"http://forum.citizen.re/\">Citizen.re</A> and mention the details below.";
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
	return false;

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
		client->Register();
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

	return false;
}

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	// step 1: write minidump
	wchar_t error[1024];
	wchar_t filename[MAX_PATH];
	__time64_t time;
	tm* ltime;

	CreateDirectory(MakeRelativeCitPath(L"crashes").c_str(), nullptr);

	_time64(&time);
	ltime = _localtime64(&time);
	wcsftime(filename, _countof(filename) - 1, MakeRelativeCitPath(L"crashes\\crash-%Y%m%d%H%M%S.dmp").c_str(), ltime);
	_snwprintf(error, _countof(error) - 1, L"A minidump has been written to %s.", filename);

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION ex;
		memset(&ex, 0, sizeof(ex));
		ex.ThreadId = GetCurrentThreadId();
		ex.ExceptionPointers = ExceptionInfo;
		ex.ClientPointers = FALSE;

		if (FAILED(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ex, NULL, NULL)))
		{
			_snwprintf(error, _countof(error) - 1, L"An error (0x%X) occurred during writing %s.", GetLastError(), filename);
		}

		CloseHandle(hFile);
	}
	else
	{
		_snwprintf(error, _countof(error) - 1, L"An error (0x%X) occurred during creating %s.", GetLastError(), filename);
	}

	// step 2: exit the application
	wchar_t errorCode[1024];
	wsprintf(errorCode, L"Fatal error (0x%08X) at 0x%08X.\n%s", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress, error);
	
	HWND wnd = nullptr;

#ifdef GTA_NY
	wnd = *(HWND*)0x1849DDC;
#endif

	MessageBox(wnd, errorCode, L"CitizenFX Fatal Error", MB_OK | MB_ICONSTOP);
	ExitProcess(1);

	return 0;
}