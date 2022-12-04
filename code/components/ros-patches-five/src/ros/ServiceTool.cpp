/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ToolComponentHelpers.h"

#include <Hooking.h>
#include <CL2LaunchMode.h>

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
		auto hEvent = CreateEventW(NULL, TRUE, FALSE, va(L"Cfx_ROSServiceEvent_%s", ToWide(launch::GetLaunchModeKey())));
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

BOOL CloseServiceHandleHook(_In_ SC_HANDLE hSCObject)
{
	return TRUE;
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

static HANDLE CreateNamedPipeAHook(_In_ LPCSTR lpName, _In_ DWORD dwOpenMode, _In_ DWORD dwPipeMode, _In_ DWORD nMaxInstances, _In_ DWORD nOutBufferSize, _In_ DWORD nInBufferSize, _In_ DWORD nDefaultTimeOut, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if (strstr(lpName, "MTLService"))
	{
		lpName = va("\\\\.\\pipe\\MTLService_Pipe_CFX%s", IsCL2() ? "_CL2" : "");
	}

	return CreateNamedPipeA(lpName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize, nDefaultTimeOut, lpSecurityAttributes);
}

HANDLE CreateFileMappingAHook(_In_ HANDLE hFile, _In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes, _In_ DWORD flProtect, _In_ DWORD dwMaximumSizeHigh, _In_ DWORD dwMaximumSizeLow, _In_opt_ LPCSTR lpName)
{
	if (lpName && strstr(lpName, "MTLService_FileMapping"))
	{
		lpName = va("MTLService_FileMapping_CFX%s", IsCL2() ? "_CL2" : "");
	}

	return CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
}

BOOL SetFileSecurityWHook(LPCWSTR lpFileName, SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
	SetLastError(0);
	return TRUE;
}

BOOL AccessCheckHook( _In_ PSECURITY_DESCRIPTOR pSecurityDescriptor, _In_ HANDLE ClientToken, _In_ DWORD DesiredAccess, _In_ PGENERIC_MAPPING GenericMapping, _Out_writes_bytes_to_opt_(*PrivilegeSetLength,*PrivilegeSetLength) PPRIVILEGE_SET PrivilegeSet, _Inout_ LPDWORD PrivilegeSetLength, _Out_ LPDWORD GrantedAccess, _Out_ LPBOOL AccessStatus )
{
	*AccessStatus = TRUE;

	SetLastError(0);
	return TRUE;
}

static const auto kInvalidHKey = (HKEY)0x409;

static LSTATUS WINAPI RegCreateKeyExWStub(HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	auto result = RegCreateKeyExW(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

	if (result != ERROR_SUCCESS)
	{
		*phkResult = kInvalidHKey;
	}

	return 0;
}

static LSTATUS WINAPI RegSetValueExWStub(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData)
{
	if (hKey == kInvalidHKey)
	{
		return 0;
	}

	return RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

static LSTATUS WINAPI RegCloseKeyStub(HKEY hKey)
{
	if (hKey == kInvalidHKey)
	{
		return 0;
	}

	return RegCloseKey(hKey);
}

extern HINSTANCE __stdcall ShellExecuteWStub(_In_opt_ HWND hwnd, _In_opt_ LPCWSTR lpOperation, _In_ LPCWSTR lpFile, _In_opt_ LPCWSTR lpParameters, _In_opt_ LPCWSTR lpDirectory, _In_ INT nShowCmd);
extern LONG __stdcall WinVerifyTrustStub(HWND hwnd, GUID* pgActionID, LPVOID pWVTData);

static std::vector<std::tuple<const char*, void*, const char*>> g_serviceHooks = {
	{ "advapi32.dll", StartServiceCtrlDispatcherWHook, "StartServiceCtrlDispatcherW" },
	{ "advapi32.dll", SetServiceStatusHook, "SetServiceStatus" },
	{ "advapi32.dll", RegisterServiceCtrlHandlerExWHook, "RegisterServiceCtrlHandlerExW" },
	{ "advapi32.dll", AccessCheckHook, "AccessCheck" },
	{ "advapi32.dll", SetFileSecurityWHook, "SetFileSecurityW" },
	{ "kernel32.dll", GetCommandLineWHook, "GetCommandLineW" },
	{ "kernel32.dll", CreateNamedPipeAHook, "CreateNamedPipeA" },
	{ "kernel32.dll", CreateFileMappingAHook, "CreateFileMappingA" },
	{ "advapi32.dll", QueryServiceStatusExHook, "QueryServiceStatusEx" },
	{ "advapi32.dll", OpenServiceWHook, "OpenServiceW" },
	{ "advapi32.dll", CloseServiceHandleHook, "CloseServiceHandle" },
	{ "advapi32.dll", QueryServiceConfigWHook, "QueryServiceConfigW" },
	{ "shell32.dll", ShellExecuteWStub, "ShellExecuteW" },
	{ "wintrust.dll", WinVerifyTrustStub, "WinVerifyTrust" },
	{ "advapi32.dll", RegCreateKeyExWStub, "RegCreateKeyExW" },
	{ "advapi32.dll", RegSetValueExWStub, "RegSetValueExW" },
	{ "advapi32.dll", RegCloseKeyStub, "RegCloseKey" },
};

static FARPROC GetProcAddressHook(HMODULE hModule, LPCSTR funcName)
{
	for (const auto& h : g_serviceHooks)
	{
		if (strcmp(std::get<2>(h), funcName) == 0)
		{
			return (FARPROC)std::get<1>(h);
		}
	}

	return GetProcAddress(hModule, funcName);
}

static void Service_Run(const boost::program_options::variables_map& map)
{
	boost::filesystem::path programPath(L"C:\\Program Files\\Rockstar Games\\Launcher\\RockstarService.exe");

	auto parentPath = programPath.parent_path();
	SetCurrentDirectory(parentPath.wstring().c_str());

	trace("service! %s\n", GetCommandLineA());

	ToolMode_SetPostLaunchRoutine([]()
	{
		hook::iat("kernel32.dll", GetProcAddressHook, "GetProcAddress");

		for (const auto& h : g_serviceHooks)
		{
			hook::iat(std::get<0>(h), std::get<1>(h), std::get<2>(h));
		}
	});

	auto mutex = CreateMutexW(NULL, TRUE, va(L"Cfx_ROSServiceMutex_%s", ToWide(launch::GetLaunchModeKey())));

	g_origProcess = programPath.wstring();
	ToolMode_LaunchGame(programPath.wstring().c_str());
}

static FxToolCommand rosSubprocess("ros:service", Service_HandleArguments, Service_Run);
