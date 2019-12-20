/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ToolComponentHelpers.h"

#include <Hooking.h>

#include <boost/filesystem/path.hpp>

static void Service_HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()
		("cake", boost::program_options::value<std::vector<std::string>>()->required(), "");

	boost::program_options::positional_options_description positional;
	positional.add("cake", -1);

	parser.options(desc).
		positional(positional).
		allow_unregistered();

	cb();
}

extern std::wstring g_origProcess;

static LPHANDLER_FUNCTION_EX g_handler;

SERVICE_STATUS_HANDLE RegisterServiceCtrlHandlerExWHook(
	LPCWSTR               lpServiceName,
	LPHANDLER_FUNCTION_EX lpHandlerProc,
	LPVOID                lpContext
)
{
	g_handler = lpHandlerProc;

	return (SERVICE_STATUS_HANDLE)8192;
}

BOOL SetServiceStatusHook(
	SERVICE_STATUS_HANDLE hServiceStatus,
	LPSERVICE_STATUS      lpServiceStatus
)
{
	trace("SetServiceStatus: %d\n", lpServiceStatus->dwCurrentState);

	if (lpServiceStatus->dwCurrentState == SERVICE_RUNNING)
	{
		auto hEvent = CreateEventW(NULL, TRUE, FALSE, L"Cfx_ROSServiceEvent");
		SetEvent(hEvent);
	}

	return TRUE;
}

BOOL QueryServiceStatusExHook(
	SC_HANDLE      hService,
	SC_STATUS_TYPE InfoLevel,
	LPBYTE         lpBuffer,
	DWORD          cbBufSize,
	LPDWORD        pcbBytesNeeded
)
{
	if (InfoLevel == SC_STATUS_PROCESS_INFO)
	{
		auto rv = (SERVICE_STATUS_PROCESS*)lpBuffer;
		*pcbBytesNeeded = sizeof(SERVICE_STATUS_PROCESS);

		memset(rv, 0, sizeof(SERVICE_STATUS_PROCESS));

		rv->dwProcessId = GetCurrentProcessId();
		rv->dwCurrentState = SERVICE_START_PENDING;
	}

	return TRUE;
}

SC_HANDLE OpenServiceWHook(_In_ SC_HANDLE hSCManager, _In_ LPCWSTR lpServiceName, _In_ DWORD dwDesiredAccess)
{
	return (SC_HANDLE)4096;
}

BOOL StartServiceCtrlDispatcherWHook(_In_ CONST SERVICE_TABLE_ENTRYW* lpServiceStartTable)
{
	const wchar_t* a = L"";
	lpServiceStartTable[0].lpServiceProc(0, const_cast<LPWSTR*>(&a));

	while (TRUE)
	{
		Sleep(50);
	}

	return TRUE;
}

BOOL QueryServiceConfigWHook(_In_ SC_HANDLE hService, _Out_writes_bytes_opt_(cbBufSize) LPQUERY_SERVICE_CONFIGW lpServiceConfig, _In_ DWORD cbBufSize, _Out_ LPDWORD pcbBytesNeeded)
{
	lpServiceConfig->lpBinaryPathName = L"\"C:\\Program Files\\Rockstar Games\\Launcher\\RockstarService.exe\"";

	return TRUE;
}

LPCWSTR GetCommandLineWHook()
{
	return L"\"C:\\Program Files\\Rockstar Games\\Launcher\\RockstarService.exe\"";
}

HANDLE CreateNamedPipeAHook(_In_ LPCSTR lpName, _In_ DWORD dwOpenMode, _In_ DWORD dwPipeMode, _In_ DWORD nMaxInstances, _In_ DWORD nOutBufferSize, _In_ DWORD nInBufferSize, _In_ DWORD nDefaultTimeOut, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if (strstr(lpName, "MTLService"))
	{
		lpName = "\\\\.\\pipe\\MTLService_Pipe_CFX";
	}

	return CreateNamedPipeA(lpName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize, nDefaultTimeOut, lpSecurityAttributes);
}

HANDLE CreateFileMappingAHook(_In_ HANDLE hFile, _In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes, _In_ DWORD flProtect, _In_ DWORD dwMaximumSizeHigh, _In_ DWORD dwMaximumSizeLow, _In_opt_ LPCSTR lpName)
{
	if (lpName && strstr(lpName, "MTLService_FileMapping"))
	{
		lpName = "MTLService_FileMapping_CFX";
	}

	return CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
}

static void Service_Run(const boost::program_options::variables_map& map)
{
	boost::filesystem::path programPath(L"C:\\Program Files\\Rockstar Games\\Launcher\\RockstarService.exe");

	auto parentPath = programPath.parent_path();
	SetCurrentDirectory(parentPath.wstring().c_str());

	trace("service! %s\n", GetCommandLineA());

	ToolMode_SetPostLaunchRoutine([]()
	{
		hook::iat("advapi32.dll", StartServiceCtrlDispatcherWHook, "StartServiceCtrlDispatcherW");
		hook::iat("advapi32.dll", SetServiceStatusHook, "SetServiceStatus");
		hook::iat("advapi32.dll", RegisterServiceCtrlHandlerExWHook, "RegisterServiceCtrlHandlerExW");
		hook::iat("kernel32.dll", GetCommandLineWHook, "GetCommandLineW");
		hook::iat("kernel32.dll", CreateNamedPipeAHook, "CreateNamedPipeA");
		hook::iat("kernel32.dll", CreateFileMappingAHook, "CreateFileMappingA");
		hook::iat("advapi32.dll", QueryServiceStatusExHook, "QueryServiceStatusEx");
		hook::iat("advapi32.dll", OpenServiceWHook, "OpenServiceW");
		hook::iat("advapi32.dll", QueryServiceConfigWHook, "QueryServiceConfigW");
	});

	g_origProcess = programPath.wstring();
	ToolMode_LaunchGame(programPath.wstring().c_str());
}

static FxToolCommand rosSubprocess("ros:service", Service_HandleArguments, Service_Run);
