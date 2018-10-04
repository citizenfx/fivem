/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifndef IS_FXSERVER
#include <minhook.h>

#ifdef _M_AMD64
#pragma comment(lib, "ntdll.lib")

#include <winternl.h>

typedef wchar_t*(*MappingFunctionType)(const wchar_t*, void*(*)(size_t));

static MappingFunctionType g_mappingFunction;

static NTSTATUS(*g_origLoadDll)(const wchar_t*, uint32_t*, UNICODE_STRING*, HANDLE*);

static std::wstring MapRedirectedFilename(const wchar_t* origFileName)
{
	if (wcsstr(origFileName, L"autosignin.dat") != nullptr)
	{
		return MakeRelativeCitPath(L"cache\\game\\autosignin.dat");
	}

	if (wcsstr(origFileName, L"version.txt") != nullptr)
	{
		return MakeRelativeCitPath(L"cache\\game\\version_orig.txt");
	}

	wchar_t* fileName = g_mappingFunction(origFileName, malloc);

	std::wstring retval(fileName);
	free(fileName);

	return retval;
}

static std::wstring MapRedirectedNtFilename(const wchar_t* origFileName)
{
	std::wstring redirected = MapRedirectedFilename(origFileName);

	if (redirected.find(L"\\\\", 0) == 0)
	{
		return L"\\Device\\Mup\\" + redirected.substr(2);
	}
	else
	{
		return L"\\??\\" + redirected;
	}
}

static bool IsMappedFilename(const std::wstring& fileName)
{
	if (fileName.find(L"Files\\Rockstar Games\\Social Club") != std::string::npos)
	{
		return true;
	}
	
	if (fileName.find(L"autosignin.dat") != std::string::npos)
	{
		return true;
	}

	if (fileName.find(L"version.txt") != std::string::npos)
	{
		return true;
	}

	if (fileName.length() > 10 && fileName.compare(fileName.length() - 8, 8, L"GTA5.exe") == 0)
	{
		return true;
	}

	return false;
}

static HANDLE(*g_origCreateFileW)(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile);

static HANDLE CreateFileWStub(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return g_origCreateFileW(fileName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

NTSTATUS NTAPI LdrLoadDllStub(const wchar_t* fileName, uint32_t* flags, UNICODE_STRING* moduleName, HANDLE* handle)
{
	UNICODE_STRING newString;
	std::wstring moduleNameStr(moduleName->Buffer, moduleName->Length / sizeof(wchar_t));

	if (moduleNameStr.find(L"socialclub.dll") != std::string::npos)
	{
		moduleNameStr = MapRedirectedFilename(moduleNameStr.c_str());

		RtlInitUnicodeString(&newString, moduleNameStr.c_str());

		moduleName = &newString;
	}

	std::transform(moduleNameStr.begin(), moduleNameStr.end(), moduleNameStr.begin(), ::tolower);

	if (moduleNameStr.find(L"fraps64.dll") != std::string::npos || moduleNameStr.find(L"avghooka.dll") != std::string::npos ||
		// certain versions of RTSS crash d3d9.dll by badly patching Win10 RS2 hotpatch stubs
		moduleNameStr.find(L"rtsshooks64.dll") != std::string::npos ||
		// apparently crashes NUI
		moduleNameStr.find(L"bdcam64.dll") != std::string::npos ||
		// lots of crashes occur in the DiscordApp overlay
		moduleNameStr.find(L"overlay.x64.dll") != std::string::npos ||
		// new DiscordApp overlay name :/
		moduleNameStr.find(L"DiscordHook64.dll") != std::string::npos ||
		// HideMyIP, causes LoopbackTcpServer and libuv to crash
		moduleNameStr.find(L"hmipcore64.dll") != std::string::npos ||
		// NVIDIA SHARE/ShadowPlay capture DLL, high correlation with ERR_GFX_D3D_INIT failures
		moduleNameStr.find(L"nvspcap") != std::string::npos ||
		// Proxifier, causes LoopbackTcpServer crashes
		moduleNameStr.find(L"PrxerNsp.dll") != std::string::npos ||
		// Ad Muncher, causes LoopbackTcpServer to crash
		moduleNameStr.find(L"am64-34121.dll") != std::string::npos
	)
	{
		return 0xC0000135;
	}

	return g_origLoadDll(fileName, flags, moduleName, handle);
}

LONG (WINAPI* g_origRegOpenKeyExW)(HKEY key, const wchar_t* subKey, DWORD options, REGSAM samDesired, PHKEY outKey);

void LSP_InitializeHooks();

LONG WINAPI RegOpenKeyExWStub(HKEY key, const wchar_t* subKey, DWORD options, REGSAM samDesired, PHKEY outKey)
{
	if (subKey && wcsstr(subKey, L"Rockstar Games Social Club"))
	{
		//LONG status = RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\Social Club", 0, nullptr, 0, KEY_READ, nullptr, outKey, nullptr);
		auto setValue = [&] (const wchar_t* name, const wchar_t* keyString)
		{
			RegSetKeyValue(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\Social Club", name, REG_SZ, keyString, (wcslen(keyString) * 2) + 2);
		};

		setValue(L"InstallFolder", MakeRelativeCitPath(L"cache\\game\\ros_1238").c_str());
		setValue(L"InstallLang", L"1033");
		setValue(L"Version", L"1.2.3.8");

        LONG status = g_origRegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\Social Club", 0, KEY_READ, outKey);

		return status;
	}

	return g_origRegOpenKeyExW(key, subKey, options, samDesired, outKey);
}

static DWORD g_tlsHandle;

struct TlsState
{
	bool inCreateFile;
	bool inOpenFile;
	bool inDeleteFile;
	bool inQueryAttributes;
};

static TlsState* GetTls()
{
	auto data = TlsGetValue(g_tlsHandle);

	if (!data)
	{
		data = HeapAlloc(GetProcessHeap(), 0, sizeof(TlsState));
		memset(data, 0, sizeof(TlsState));

		TlsSetValue(g_tlsHandle, data);
	}

	return reinterpret_cast<TlsState*>(data);
}

NTSTATUS(WINAPI* g_origNtCreateFile)(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioBlock, PLARGE_INTEGER allocationSize,
									 ULONG fileAttributes, ULONG shareAccess, ULONG createDisposition, ULONG createOptions, PVOID eaBuffer, ULONG eaLength);

NTSTATUS WINAPI NtCreateFileStub(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioBlock, PLARGE_INTEGER allocationSize,
								 ULONG fileAttributes, ULONG shareAccess, ULONG createDisposition, ULONG createOptions, PVOID eaBuffer, ULONG eaLength)
{
	auto tls = GetTls();

	if (!tls->inCreateFile)
	{
		tls->inCreateFile = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		NTSTATUS retval = g_origNtCreateFile(fileHandle, desiredAccess, &attributes, ioBlock, allocationSize, fileAttributes, shareAccess, createDisposition, createOptions, eaBuffer, eaLength);

		tls->inCreateFile = false;

		return retval;
	}

	return g_origNtCreateFile(fileHandle, desiredAccess, objectAttributes, ioBlock, allocationSize, fileAttributes, shareAccess, createDisposition, createOptions, eaBuffer, eaLength);
}

NTSTATUS(WINAPI* g_origNtOpenFile)(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioBlock, ULONG shareAccess, ULONG openOptions);

NTSTATUS WINAPI NtOpenFileStub(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioBlock, ULONG shareAccess, ULONG openOptions)
{
	auto tls = GetTls();

	if (!tls->inOpenFile)
	{
		tls->inOpenFile = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		NTSTATUS retval = g_origNtOpenFile(fileHandle, desiredAccess, &attributes, ioBlock, shareAccess, openOptions);

		tls->inOpenFile = false;

		return retval;
	}

	return g_origNtOpenFile(fileHandle, desiredAccess, objectAttributes, ioBlock, shareAccess, openOptions);
}

NTSTATUS(WINAPI* g_origNtDeleteFile)(POBJECT_ATTRIBUTES objectAttributes);

NTSTATUS WINAPI NtDeleteFileStub(POBJECT_ATTRIBUTES objectAttributes)
{
	auto tls = GetTls();

	if (!tls->inDeleteFile)
	{
		tls->inDeleteFile = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		NTSTATUS retval = g_origNtDeleteFile(&attributes);

		tls->inDeleteFile = false;

		return retval;
	}

	return g_origNtDeleteFile(objectAttributes);
}

NTSTATUS(WINAPI* g_origNtQueryAttributesFile)(POBJECT_ATTRIBUTES objectAttributes, void* basicInformation);

NTSTATUS WINAPI NtQueryAttributesFileStub(POBJECT_ATTRIBUTES objectAttributes, void* basicInformation)
{
	auto tls = GetTls();

	if (!tls->inQueryAttributes)
	{
		tls->inQueryAttributes = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		NTSTATUS retval = g_origNtQueryAttributesFile(&attributes, basicInformation);

		tls->inQueryAttributes = false;

		return retval;
	}

	return g_origNtQueryAttributesFile(objectAttributes, basicInformation);
}

static HRESULT(WINAPI* g_origQueryDListForApplication1)(BOOL* pDefaultToDiscrete, HANDLE hAdapter, void* pfnEscapeCB);

static HRESULT WINAPI QueryDListForApplication1Stub(BOOL* pDefaultToDiscrete, HANDLE hAdapter, void* pfnEscapeCB)
{
	auto hr = g_origQueryDListForApplication1(pDefaultToDiscrete, hAdapter, pfnEscapeCB);

	if (SUCCEEDED(hr))
	{
		// log if done
		if (!*pDefaultToDiscrete)
		{
			trace("Overriding QueryDListForApplication1 to run on discrete GPU.\n");
		}

		// we always want to run on the discrete GPU, even if the UMD says not to
		*pDefaultToDiscrete = TRUE;
	}

	return hr;
}

static FARPROC(WINAPI* g_origGetProcAddress)(HMODULE hModule, LPCSTR procName, void* caller);

FARPROC WINAPI GetProcAddressStub(HMODULE hModule, LPCSTR lpProcName, void* caller)
{
	static const DWORD bigOne = 1;

	if (!IS_INTRESOURCE(lpProcName))
	{
		if (_stricmp(lpProcName, "NvOptimusEnablement") == 0 || _stricmp(lpProcName, "AmdPowerXpressRequestHighPerformance") == 0)
		{
			return (FARPROC)&bigOne;
		}
	}

	return g_origGetProcAddress(hModule, lpProcName, caller);
}

extern "C" BOOLEAN NTAPI RtlEqualString(
	_In_ const STRING  *String1,
	_In_ const STRING  *String2,
	_In_       BOOLEAN CaseInSensitive
);

extern "C" BOOLEAN NTAPI RtlEqualUnicodeString(
	_In_ const UNICODE_STRING  *String1,
	_In_ const UNICODE_STRING  *String2,
	_In_       BOOLEAN CaseInSensitive
);

static NTSTATUS(NTAPI* g_origGetProcedureAddress)(HMODULE hModule, PANSI_STRING functionName, WORD ordinal, PVOID* fnAddress);

NTSTATUS NTAPI LdrGetProcedureAddressStub(HMODULE hModule, PANSI_STRING functionName, WORD ordinal, PVOID* fnAddress)
{
	static const DWORD bigOne = 1;

	if (functionName)
	{
		ANSI_STRING compareNv, compareAmd;
		RtlInitAnsiString(&compareNv, "NvOptimusEnablement");
		RtlInitAnsiString(&compareAmd, "AmdPowerXpressRequestHighPerformance");

		if (RtlEqualString(functionName, &compareNv, TRUE) || RtlEqualString(functionName, &compareAmd, TRUE))
		{
			*fnAddress = const_cast<DWORD*>(&bigOne);
			return 0;
		}
	}

	return g_origGetProcedureAddress(hModule, functionName, ordinal, fnAddress);
}

#define LDR_DLL_NOTIFICATION_REASON_LOADED 1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA
{
	ULONG Flags;
	PUNICODE_STRING FullDllName;
	PUNICODE_STRING BaseDllName;
	PVOID DllBase;
	ULONG SizeOfImage;
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA
{
	ULONG Flags;
	PCUNICODE_STRING FullDllName;
	PCUNICODE_STRING BaseDllName;
	PVOID DllBase;
	ULONG SizeOfImage;
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA
{
	LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
	LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;

typedef VOID(NTAPI *PLDR_DLL_NOTIFICATION_FUNCTION)(
	_In_ ULONG NotificationReason,
	_In_ PLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_ PVOID Context
	);

NTSYSAPI
NTSTATUS
NTAPI
LdrRegisterDllNotification(
	_In_ ULONG Flags,
	_In_ PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
	_In_ PVOID Context,
	_Out_ PVOID *Cookie
);

static PVOID g_lastDllNotif;

VOID CALLBACK LdrDllNotification(
	_In_     ULONG                       NotificationReason,
	_In_     PLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_ PVOID                       Context
)
{
	if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED)
	{
		UNICODE_STRING amdHdl;
		RtlInitUnicodeString(&amdHdl, L"amdhdl64.dll");

		if (RtlEqualUnicodeString(NotificationData->Loaded.BaseDllName, &amdHdl, TRUE))
		{
			void* proc = GetProcAddress(reinterpret_cast<HMODULE>(NotificationData->Loaded.DllBase), "QueryDListForApplication1");
			MH_CreateHook(proc, QueryDListForApplication1Stub, (void**)&g_origQueryDListForApplication1);
			MH_EnableHook(proc);

			trace("Hooked amdhdl64.dll.\n");
		}
	}
}

extern "C" DLL_EXPORT void CoreSetMappingFunction(MappingFunctionType function)
{
	g_mappingFunction = function;
	g_tlsHandle = TlsAlloc();

	MH_CreateHookApi(L"ntdll.dll", "NtCreateFile", NtCreateFileStub, (void**)&g_origNtCreateFile);
	MH_CreateHookApi(L"ntdll.dll", "NtOpenFile", NtOpenFileStub, (void**)&g_origNtOpenFile);
	MH_CreateHookApi(L"ntdll.dll", "NtDeleteFile", NtDeleteFileStub, (void**)&g_origNtDeleteFile);
	MH_CreateHookApi(L"ntdll.dll", "NtQueryAttributesFile", NtQueryAttributesFileStub, (void**)&g_origNtQueryAttributesFile);
	MH_CreateHookApi(L"ntdll.dll", "LdrLoadDll", LdrLoadDllStub, (void**)&g_origLoadDll);
	MH_CreateHookApi(L"ntdll.dll", "LdrGetProcedureAddress", LdrGetProcedureAddressStub, (void**)&g_origGetProcedureAddress);
	MH_CreateHookApi(L"kernelbase.dll", "RegOpenKeyExW", RegOpenKeyExWStub, (void**)&g_origRegOpenKeyExW);
	MH_CreateHookApi(L"kernelbase.dll", "GetProcAddressForCaller", GetProcAddressStub, (void**)&g_origGetProcAddress);
	MH_EnableHook(MH_ALL_HOOKS);

	static auto _LdrRegisterDllNotification = (decltype(&LdrRegisterDllNotification))GetProcAddress(GetModuleHandle(L"ntdll.dll"), "LdrRegisterDllNotification");
	_LdrRegisterDllNotification(0, LdrDllNotification, nullptr, &g_lastDllNotif);

    LSP_InitializeHooks();

	trace("Initialized system mapping!\n");
}
#endif
#endif
