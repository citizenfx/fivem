#include <StdInc.h>

#ifndef IS_FXSERVER
#include <minhook.h>

static LONG(WINAPI* g_origRegOpenKeyExA)(HKEY key, const char* subKey, DWORD options, REGSAM samDesired, PHKEY outKey);

LONG WINAPI ProcessLSPRegOpenKeyExA(HKEY key, const char* subKey, DWORD options, REGSAM samDesired, PHKEY outKey)
{
    static thread_local HKEY lastLSPKey = (HKEY)-1;

    if (subKey)
    {
        if (!_stricmp(subKey, "AppId_Catalog"))
        {
            auto setValue = [&](const wchar_t* name, const wchar_t* keyString)
            {
                RegSetKeyValue(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\AppID_Catalog", name, REG_SZ, keyString, (wcslen(keyString) * 2) + 2);
            };

            wchar_t modulePath[512];
            GetModuleFileName(GetModuleHandle(nullptr), modulePath, sizeof(modulePath) / sizeof(wchar_t));

            setValue(L"AppFullPath", modulePath);
            
            DWORD permittedCategories = 0;
            RegSetKeyValue(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\AppID_Catalog", L"PermittedLspCategories", REG_DWORD, &permittedCategories, sizeof(permittedCategories));

            LONG status = g_origRegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\CitizenFX\\AppID_Catalog", options, samDesired, outKey);
            lastLSPKey = *outKey;

            return status;
        }
    }

    if (key == lastLSPKey)
    {
        if (!strchr(subKey, L'-'))
        {
            LONG status = g_origRegOpenKeyExA(key, "", options, samDesired, outKey);

            lastLSPKey = (HKEY)-1;

            return status;
        }
    }

    return g_origRegOpenKeyExA(key, subKey, options, samDesired, outKey);
}

#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

static void* origQIP;
static DWORD explorerPid;

#include <ntstatus.h>

#define DEFINE_MODULE_CHECK(key, name) \
	struct key##Module { static inline const wchar_t* GetName() { return L##name; }	};

DEFINE_MODULE_CHECK(Base, "kernelbase.dll");

struct NullModule { static inline const wchar_t* GetName() { return nullptr; } };

template<typename TModule>
bool IsModule(void* address)
{
	static char* g_module;
	static char* g_moduleEnd;

	if (!g_module)
	{
		g_module = (char*)GetModuleHandle(TModule::GetName());

		if (g_module)
		{
			PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)g_module;
			PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)(g_module + dosHeader->e_lfanew);

			g_moduleEnd = g_module + ntHeader->OptionalHeader.SizeOfImage;
		}
	}

	return (address >= g_module && address <= g_moduleEnd);
}

typedef NTSTATUS(*NtQueryInformationProcessType)(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL);

NTSTATUS NtQueryInformationProcessHook(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL)
{
	NTSTATUS status = ((NtQueryInformationProcessType)origQIP)(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

	if (NT_SUCCESS(status))
	{
		if (ProcessInformationClass == ProcessBasicInformation)
		{
			((PPROCESS_BASIC_INFORMATION)ProcessInformation)->Reserved3 = (PVOID)explorerPid;
		}
		else if (ProcessInformationClass == 30) // ProcessDebugObjectHandle
		{
			*(HANDLE*)ProcessInformation = 0;

			return STATUS_PORT_NOT_SET;
		}
		else if (ProcessInformationClass == 7) // ProcessDebugPort
		{
			if (!IsModule<BaseModule>(_ReturnAddress()))
			{
				*(HANDLE*)ProcessInformation = 0;
			}
		}
		else if (ProcessInformationClass == 31)
		{
			*(ULONG*)ProcessInformation = 1;
		}
	}

	return status;
}

typedef NTSTATUS(*NtCloseType)(IN HANDLE Handle);
static NtCloseType origClose;

typedef struct _OBJECT_HANDLE_ATTRIBUTE_INFORMATION
{
	BOOLEAN Inherit;
	BOOLEAN ProtectFromClose;
} OBJECT_HANDLE_ATTRIBUTE_INFORMATION, *POBJECT_HANDLE_ATTRIBUTE_INFORMATION;

NTSTATUS NTAPI NtCloseHook(IN HANDLE Handle)
{
	char info[16];

	if (NtQueryObject(Handle, (OBJECT_INFORMATION_CLASS)4, &info, sizeof(OBJECT_HANDLE_ATTRIBUTE_INFORMATION), nullptr) >= 0)
	{
		DWORD flags = 0;

		if (GetHandleInformation(Handle, &flags) && (flags & HANDLE_FLAG_PROTECT_FROM_CLOSE) != 0)
		{
			return STATUS_SUCCESS;
		}

		origClose(Handle);
		return STATUS_SUCCESS;
	}

	return STATUS_SUCCESS;
}


void LSP_InitializeHooks()
{
	HWND shellWindow = GetShellWindow();
	GetWindowThreadProcessId(shellWindow, &explorerPid);

	MH_CreateHookApi(L"kernelbase.dll", "RegOpenKeyExA", ProcessLSPRegOpenKeyExA, (void**)&g_origRegOpenKeyExA);

	//if (CoreIsDebuggerPresent())
	{
		//MH_CreateHookApi(L"ntdll.dll", "NtQueryInformationProcess", NtQueryInformationProcessHook, (void**)&origQIP);
		//MH_CreateHookApi(L"ntdll.dll", "NtClose", NtCloseHook, (void**)&origClose);
		origQIP = (decltype(origQIP))GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQueryInformationProcess");
		origClose = (decltype(origClose))GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtClose");
	}

	MH_EnableHook(MH_ALL_HOOKS);
}
#endif
