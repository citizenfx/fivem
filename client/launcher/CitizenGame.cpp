/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CitizenGame.h"
#include "ExecutableLoader.h"
#include "LauncherInterface.h"

#include "Hooking.h"

#include <winternl.h>

#if defined(PAYNE)
BYTE g_gmfOrig[5];
BYTE g_gmfOrigW[5];
BYTE g_gsiOrig[5];

ILauncherInterface* g_launcher;

bool ThisIsActuallyLaunchery()
{
	return true;
}

VOID WINAPI GetStartupInfoAHook(LPSTARTUPINFOA startupInfo)
{
	// put back the original and call it
	memcpy(GetStartupInfoA, g_gsiOrig, 5);

	GetStartupInfoA(startupInfo);

	// analyze the return address for a recommended course of action (GameShield is using a Borland-style compiler, so the only VC CRT here will be Payne)
	char* charData = (char*)_ReturnAddress();

	if (!memcmp(charData, "\x6A\xFE\x5F\x89\x7D\xFC\xB8\x4D", 8))
	{
		PEXCEPTION_REGISTRATION_RECORD sehPtr = (PEXCEPTION_REGISTRATION_RECORD)__readfsdword(0);

		// and we don't want crt init to use its own exception handler either
		__writefsdword(0, (DWORD)sehPtr->Next->Next);

		// we also need to unVP the read-only data segments
		DWORD oldProtect;

		VirtualProtect(GetModuleHandle(nullptr), 0x119e000, PAGE_EXECUTE_READWRITE, &oldProtect);

		// this is here *temporarily*
		hook::jump(hook::pattern("81 EC 44 02 00 00 55 56 33 F6 33 C0").count(1).get(0).get<void>(), ThisIsActuallyLaunchery);

		// mhm
		memcpy(GetModuleFileNameA, g_gmfOrig, 5);
		memcpy(GetModuleFileNameW, g_gmfOrigW, 5);

		if (!g_launcher->PostLoadGame(GetModuleHandle(nullptr), nullptr))
		{
			ExitProcess(0);
		}

		// so it can pop itself
		//__writefsdword(0, (DWORD)sehPtr);
	}
	else
	{
		// hook us back in for the next pass
		hook::jump((uintptr_t)GetStartupInfoA, GetStartupInfoAHook);
	}
}

DWORD WINAPI GetModuleFileNameAHook(HMODULE hModule, LPSTR lpFileName, DWORD nSize)
{
	uintptr_t module = (uintptr_t)hModule;
	DWORD result = 0;

	if (module == 0x400000 || module == 0)
	{
		wcstombs(lpFileName, MakeRelativeGamePath(L"\\" GAME_EXECUTABLE).c_str(), nSize);

		result = strlen(lpFileName);
	}
	else
	{
		memcpy(GetModuleFileNameA, g_gmfOrig, 5);

		result = GetModuleFileNameA(hModule, lpFileName, nSize);

		hook::jump((uintptr_t)GetModuleFileNameA, GetModuleFileNameAHook);
	}

	return result;
}

DWORD WINAPI GetModuleFileNameWHook(HMODULE hModule, LPWSTR lpFileName, DWORD nSize)
{
	uintptr_t module = (uintptr_t)hModule;
	DWORD result = 0;

	if (module == 0x400000 || module == 0)
	{
		wcsncpy(lpFileName, MakeRelativeGamePath(L"\\" GAME_EXECUTABLE).c_str(), nSize);

		result = wcslen(lpFileName);
	}
	else
	{
		memcpy(GetModuleFileNameW, g_gmfOrigW, 5);

		result = GetModuleFileNameW(hModule, lpFileName, nSize);

		hook::jump((uintptr_t)GetModuleFileNameW, GetModuleFileNameWHook);
	}

	return result;
}
#endif

static LONG NTAPI HandleVariant(PEXCEPTION_POINTERS exceptionInfo)
{
	return (exceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INVALID_HANDLE) ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
}

void CitizenGame::InvokeEntryPoint(void(*entryPoint)())
{
	// SEH call to prevent STATUS_INVALID_HANDLE
	__try
	{
		// and call the entry point
		entryPoint();
	}
	__except (HandleVariant(GetExceptionInformation()))
	{

	}
}

#if defined(GTA_FIVE)
static void* origCloseHandle;

typedef struct _OBJECT_HANDLE_ATTRIBUTE_INFORMATION
{
	BOOLEAN Inherit;
	BOOLEAN ProtectFromClose;
} OBJECT_HANDLE_ATTRIBUTE_INFORMATION, *POBJECT_HANDLE_ATTRIBUTE_INFORMATION;

#pragma comment(lib, "ntdll.lib")

struct NtCloseHook : public jitasm::Frontend
{
	NtCloseHook()
	{

	}

	static NTSTATUS ValidateHandle(HANDLE handle)
	{
		OBJECT_HANDLE_ATTRIBUTE_INFORMATION info;

		if (NtQueryObject(handle, (OBJECT_INFORMATION_CLASS)4, &info, sizeof(info), nullptr) >= 0)
		{
			return 0;
		}
		else
		{
			return STATUS_INVALID_HANDLE;
		}
	}

	void InternalMain()
	{
		push(rcx);

		mov(rax, (uint64_t)&ValidateHandle);
		call(rax);

		pop(rcx);

		cmp(eax, STATUS_INVALID_HANDLE);
		je("doReturn");

		mov(rax, (uint64_t)origCloseHandle);
		push(rax); // to return here, as there seems to be no jump-to-rax in jitasm

		L("doReturn");
		ret();
	}
};

void HookHandleClose()
{
	// hook NtClose (STATUS_INVALID_HANDLE debugger detection)
	uint8_t* code = (uint8_t*)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtClose");
	
	origCloseHandle = VirtualAlloc(nullptr, 20, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy(origCloseHandle, code, 20);

	NtCloseHook* hook = new NtCloseHook;
	hook->Assemble();

	DWORD oldProtect;
	VirtualProtect(code, 15, PAGE_EXECUTE_READWRITE, &oldProtect);

	*(uint8_t*)code = 0x48;
	*(uint8_t*)(code + 1) = 0xb8;

	*(uint64_t*)(code + 2) = (uint64_t)hook->GetCode();

	*(uint16_t*)(code + 10) = 0xE0FF;
}

static void* origQIP;
static DWORD explorerPid;

#include <ntstatus.h>

typedef NTSTATUS(*NtQueryInformationProcessType)(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL);

static NTSTATUS NtQueryInformationProcessHook(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL)
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
			*(HANDLE*)ProcessInformation = 0;
		}
		else if (ProcessInformationClass == 31)
		{
			*(ULONG*)ProcessInformation = 1;
		}
	}

	return status;
}

void HookQueryInformationProcess()
{
	uint8_t* code = (uint8_t*)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQueryInformationProcess");

	HWND shellWindow = GetShellWindow();
	GetWindowThreadProcessId(shellWindow, &explorerPid);

	origQIP = VirtualAlloc(nullptr, 20, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy(origQIP, code, 20);

	/*NtQueryInformationProcessHook* hook = new NtQueryInformationProcessHook;
	hook->Assemble();*/

	DWORD oldProtect;
	VirtualProtect(code, 15, PAGE_EXECUTE_READWRITE, &oldProtect);

	*(uint8_t*)code = 0x48;
	*(uint8_t*)(code + 1) = 0xb8;

	*(uint64_t*)(code + 2) = (uint64_t)NtQueryInformationProcessHook;

	*(uint16_t*)(code + 10) = 0xE0FF;
}
#endif

void CitizenGame::Launch(std::wstring& gamePath)
{
	// initialize the CEF sandbox
	void* sandboxInfo = nullptr;

	// load the game library
	HMODULE gameLibrary = LoadLibrary(MakeRelativeCitPath(L"CitizenGame.dll").c_str());

	if (!gameLibrary)
	{
		FatalError("Could not load CitizenGame.dll.");
		return;
	}

	// get the launcher interface
	GetLauncherInterface_t getLauncherInterface = (GetLauncherInterface_t)GetProcAddress(gameLibrary, "GetLauncherInterface");

	if (!getLauncherInterface)
	{
		return;
	}

	ILauncherInterface* launcher = getLauncherInterface();
	
	// call into the launcher code
	if (!launcher->PreLoadGame(sandboxInfo))
	{
		ExitProcess(0);
	}

	// load the game executable data in temporary memory
	FILE* gameFile = _wfopen(gamePath.c_str(), L"rb");
	
	if (!gameFile)
	{
		return;
	}

	// find the file length and allocate a related buffer
	uint32_t length;
	uint8_t* data;

	fseek(gameFile, 0, SEEK_END);
	length = ftell(gameFile);

	data = new uint8_t[length];

	// seek back to the start and read the file
	fseek(gameFile, 0, SEEK_SET);
	fread(data, 1, length, gameFile);

	// close the file, and continue on
	fclose(gameFile);

	// load the executable into our module context
	HMODULE exeModule = GetModuleHandle(NULL);

	ExecutableLoader exeLoader(data);
#if defined(GTA_NY)
	exeLoader.SetLoadLimit(0xF0D000);
#elif defined(PAYNE)
	exeLoader.SetLoadLimit(0x20000000);
#elif defined(GTA_FIVE)
	exeLoader.SetLoadLimit(0x140000000 + 0x60000000);
#else
#error No load limit defined.
#endif
	exeLoader.SetLibraryLoader([] (const char* libName)
	{
		if (!_stricmp(libName, "xlive.dll"))
		{
			return (HMODULE)INVALID_HANDLE_VALUE;
		}

		if (!_stricmp(libName, "d3d9.dll"))
		{
			std::wstring gameDll = MakeRelativeGamePath(L"d3d9.dll");

			if (GetFileAttributes(gameDll.c_str()) != INVALID_FILE_ATTRIBUTES)
			{
				return LoadLibrary(gameDll.c_str());
			}
		}

		// ATL80.dll is SxS, but it's unused by the game
		if (!_stricmp(libName, "atl80.dll"))
		{
			return (HMODULE)INVALID_HANDLE_VALUE;
		}

		return LoadLibraryA(libName);
	});

	exeLoader.LoadIntoModule(exeModule);

	// overwrite our header
#if defined(PAYNE)
	memset(exeModule, 0, 0x1000);
	memcpy(exeModule, data, 0x200);
#endif
	
	// free the old binary
	delete[] data;

	// call into the launcher interface
	void(*entryPoint)();

	entryPoint = (void(*)())exeLoader.GetEntryPoint();

#if !defined(PAYNE)
	if (!launcher->PostLoadGame(exeModule, &entryPoint))
	{
		ExitProcess(0);
	}
#endif

#if defined(GTA_NY)
	// apply memory protection
	DWORD oldProtect;
	VirtualProtect((void*)0x401000, 0x94C000, PAGE_EXECUTE_READ, &oldProtect); // .text
	VirtualProtect((void*)0xD4D000, 0x1BF000, PAGE_READONLY, &oldProtect); // .idata/.rdata
#endif

#if defined(PAYNE)
	// hook GetModuleFileNameA/W (in a bit of an ugly way) - GameShield reads some verification from here
	DWORD oldProtect;

	memcpy(g_gmfOrig, GetModuleFileNameA, 5);

	VirtualProtect(GetModuleFileNameA, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	hook::jump((uintptr_t)GetModuleFileNameA, GetModuleFileNameAHook);

	memcpy(g_gmfOrigW, GetModuleFileNameW, 5);

	VirtualProtect(GetModuleFileNameW, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	hook::jump((uintptr_t)GetModuleFileNameW, GetModuleFileNameWHook);

	// hook GetStartupInfoA (Payne's CRT uses this as first library call after deprotecting)
	memcpy(g_gsiOrig, GetStartupInfoA, 5);

	VirtualProtect(GetStartupInfoA, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
	hook::jump((uintptr_t)GetStartupInfoA, GetStartupInfoAHook);

	// GameShield ends up breaking the OEP if being debugged - it uses this for a check
	PPEB peb = (PPEB)__readfsdword(0x30);
	peb->BeingDebugged = false;

	// store the launcher interface
	g_launcher = launcher;
#endif

#if defined(GTA_FIVE)
	// set BeingDebugged
	PPEB peb = (PPEB)__readgsqword(0x60);
	peb->BeingDebugged = false;

	// set GlobalFlags
	*(DWORD*)((char*)peb + 0xBC) &= ~0x70;

	//MessageBox(nullptr, L"a", L"a", MB_OK);

	HookHandleClose();
	HookQueryInformationProcess();
	
	//uint8_t* code = (uint8_t*)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "KiUserExceptionDispatcher");
	/*uint8_t* code = (uint8_t*)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlLookupFunctionEntry");
	memcpy(origCode, code, sizeof(origCode));

	RtlRaiseStatus = (uint64_t)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlRaiseStatus");

	procAddr = (uint64_t)code;
	//procAddr += 0x28;
	procAddr += 0xE;

	DWORD oldProtect;
	VirtualProtect(code, 15, PAGE_EXECUTE_READWRITE, &oldProtect);

	*(uint8_t*)code = 0x48;
	*(uint8_t*)(code + 1) = 0xb8;

	*(uint64_t*)(code + 2) = (uint64_t)ExceptionHandler;
	//*(uint64_t*)(code + 2) = (uint64_t)0;

	*(uint16_t*)(code + 10) = 0xE0FF;*/
#endif

	AddVectoredExceptionHandler(0, HandleVariant);

#ifndef _M_AMD64
	return InvokeEntryPoint(entryPoint);
#else
	return entryPoint();
#endif
}