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
#include "UserLibrary.h"

#include "Hooking.h"

#include <winternl.h>

#include <Error.h>

static ILauncherInterface* g_launcher;

#if defined(PAYNE)
BYTE g_gmfOrig[5];
BYTE g_gmfOrigW[5];
BYTE g_gsiOrig[5];

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
#elif defined(GTA_FIVE)
static bool ThisIsActuallyLaunchery()
{
	return true;
}

template<int Value>
int ReturnInt()
{
	return Value;
}

static int CustomGameElementCall(char* element)
{
	static std::map<uint32_t, std::string> hashMap;

	if (hashMap.size() == 0)
	{
		FILE* f = fopen("Y:\\dev\\v\\strings2.txt", "r");

		if (f)
		{
			char stringBuf[4096];

			while (!feof(f))
			{
				fgets(stringBuf, sizeof(stringBuf), f);
				stringBuf[sizeof(stringBuf) - 1] = '\0';

				stringBuf[strlen(stringBuf) - 2] = '\0';

				hashMap[HashString(stringBuf)] = stringBuf;
			}
		}
	}

	uint32_t hash = *(uint32_t*)(element + 16);

	std::string name;

	auto it = hashMap.find(hash);

	if (it != hashMap.end())
	{
		name = " - " + it->second;
	}

	trace("Entered game element %08x%s.\n", hash, name.c_str());

	uintptr_t func = *(uintptr_t*)(element + 32);
	int retval = ((int(*)())func)();

	trace("Exited game element %08x%s.\n", hash, name.c_str());

	return retval;
}

static void* DeleteVideo(void*, char* videoName)
{
	strcpy(videoName, "nah");
	return nullptr;
}

bool g_ranStartupInfo;

VOID WINAPI GetStartupInfoWHook(_Out_ LPSTARTUPINFOW lpStartupInfo)
{
	GetStartupInfoW(lpStartupInfo);

	if (g_ranStartupInfo)
	{
		return;
	}

	g_ranStartupInfo = true;

	if (getenv("CitizenFX_ToolMode"))
	{
		auto plRoutine = (void(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "ToolMode_RunPostLaunchRoutine");
		plRoutine();

		return;
	}

	// ignore launcher requirement
	hook::call(hook::pattern("E8 ? ? ? ? 84 C0 75 ? B2 01 B9 2F A9 C2 F4").count(1).get(0).get<void>(), ThisIsActuallyLaunchery);

	// ignore steam requirement
	/*auto pattern = hook::pattern("FF 15 ? ? ? ? 84 C0 74 0C B2 01 B9 91 32 25");// 31 E8");
	if (pattern.size() > 0)
	{
		hook::nop(pattern.get(0).get<void>(0), 6);
		hook::put<uint8_t>(pattern.get(0).get<void>(8), 0xEB);
	}*/

	// ignore loading 'videos'
	hook::call(hook::get_pattern("8B F8 48 85 C0 74 47 48 8B C8 E8 ? ? ? ? 4C", -6), DeleteVideo);

	// game elements for crash handling purposes
	char* vtablePtrLoc = hook::pattern("41 89 40 10 49 83 60 18 00 48 8D 05").count(1).get(0).get<char>(12);
	void* vtablePtr = (void*)(*(int32_t*)vtablePtrLoc + vtablePtrLoc + 4);

	//hook::put(&((uintptr_t*)vtablePtr)[1], CustomGameElementCall);

	if (!g_launcher->PostLoadGame(GetModuleHandle(nullptr), nullptr))
	{
		ExitProcess(0);
	}

	if (!g_launcher->PreResumeGame())
	{
		ExitProcess(0);
	}
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
		char info[16];

		if (NtQueryObject(handle, (OBJECT_INFORMATION_CLASS)4, &info, sizeof(OBJECT_HANDLE_ATTRIBUTE_INFORMATION), nullptr) >= 0)
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

class NtdllHooks
{
private:
	UserLibrary m_ntdll;

private:
	void HookHandleClose();

public:
	NtdllHooks(const wchar_t* ntdllPath);

	void Install();
};

NtdllHooks::NtdllHooks(const wchar_t* ntdllPath)
	: m_ntdll(ntdllPath)
{
}

void NtdllHooks::Install()
{
	HookHandleClose();
}

void NtdllHooks::HookHandleClose()
{
	// hook NtClose (STATUS_INVALID_HANDLE debugger detection)
	uint8_t* code = (uint8_t*)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtClose");

	origCloseHandle = VirtualAlloc(nullptr, 1024, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy(origCloseHandle, m_ntdll.GetExportCode("NtClose"), 1024);

	NtCloseHook* hook = new NtCloseHook;
	hook->Assemble();

	DWORD oldProtect;
	VirtualProtect(code, 15, PAGE_EXECUTE_READWRITE, &oldProtect);

	*(uint8_t*)code = 0x48;
	*(uint8_t*)(code + 1) = 0xb8;

	*(uint64_t*)(code + 2) = (uint64_t)hook->GetCode();

	*(uint16_t*)(code + 10) = 0xE0FF;
}

static int NoWindowsHookExA(int, HOOKPROC, HINSTANCE, DWORD)
{
	return 1;
}

extern std::map<std::string, std::string> g_redirectionData;

static std::wstring MapRedirectedFilename(const wchar_t* lpFileName)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	// original filename with backslashes converted to slashes
	std::wstring origFileName = lpFileName;
	std::replace(origFileName.begin(), origFileName.end(), L'\\', L'/');

	std::string fileName = converter.to_bytes(origFileName);

	for (auto& redirectedPair : g_redirectionData)
	{
		if (fileName.length() >= redirectedPair.first.length())
		{
			size_t start = fileName.length() - redirectedPair.first.length();

			// FIXME: what if this is actually a UTF-8 subfilename?
			if (_stricmp(fileName.substr(start).c_str(), redirectedPair.first.c_str()) == 0)
			{
				fileName = redirectedPair.second;

				origFileName = converter.from_bytes(fileName);

				return origFileName;
			}
		}
	}

	return lpFileName;
}

static HANDLE CreateFileWHook(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return CreateFileW(fileName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static DWORD GetFileAttributesWHook(_In_ LPCWSTR lpFileName)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return GetFileAttributesW(fileName.c_str());
}

static BOOL GetFileAttributesExWHook(_In_ LPCWSTR lpFileName, _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId, _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return GetFileAttributesExW(fileName.c_str(), fInfoLevelId, lpFileInformation);
}
#endif

void CitizenGame::SetCoreMapping()
{
    auto CoreSetMappingFunction = (void(*)(wchar_t*(*)(const wchar_t*, void*(*)(size_t))))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreSetMappingFunction");

    if (CoreSetMappingFunction)
    {
        CoreSetMappingFunction([](const wchar_t* fileName, void*(*allocator)(size_t))
        {
            auto outName = MapRedirectedFilename(fileName);
            wchar_t* outString = reinterpret_cast<wchar_t*>(allocator((outName.length() + 1) * sizeof(wchar_t)));

            wcscpy(outString, outName.c_str());

            return outString;
        });
    }
}

void CitizenGame::Launch(const std::wstring& gamePath)
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

	CoreSetDebuggerPresent();

    SetCoreMapping();

	// get the launcher interface
	GetLauncherInterface_t getLauncherInterface = (GetLauncherInterface_t)GetProcAddress(gameLibrary, "GetLauncherInterface");

	if (!getLauncherInterface)
	{
		return;
	}

	ILauncherInterface* launcher = getLauncherInterface();

	if (!launcher->PreInitializeGame())
	{
		ExitProcess(0);
	}
	
	// call into the launcher code
	if (!launcher->PreLoadGame(sandboxInfo))
	{
		ExitProcess(0);
	}

	// load the game executable data in temporary memory
	FILE* gameFile = _wfopen(MapRedirectedFilename(gamePath.c_str()).c_str(), L"rb");
	
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

		if (!_stricmp(libName, "xinput1_3.dll"))
		{
			HMODULE hm = LoadLibrary(L"xinput1_4.dll");
			
			if (hm)
			{
				return hm;
			}
		}

		if (!_stricmp(libName, "d3dcompiler_43.dll"))
		{
			HMODULE hm = LoadLibrary(L"d3dcompiler_47.dll");

			if (hm)
			{
				return hm;
			}
		}

		if (!_stricmp(libName, "gfsdk_shadowlib.win64.dll"))
		{
			return LoadLibrary(MakeRelativeCitPath(L"bin/gfsdk_shadowlib.dll").c_str());
		}

		// ATL80.dll is SxS, but it's unused by the game
		if (!_stricmp(libName, "atl80.dll"))
		{
			return (HMODULE)INVALID_HANDLE_VALUE;
		}

		if (_stricmp(libName, "libcef.dll") == 0)
		{
			if (getenv("CitizenFX_ToolMode"))
			{
				return LoadLibraryW(MapRedirectedFilename(L"Social Club/libcef.dll").c_str());
			}
		}

		return LoadLibraryA(libName);
	});

	exeLoader.SetFunctionResolver([] (HMODULE module, const char* functionName) -> LPVOID
	{
#if defined(GTA_FIVE)
		if (!_stricmp(functionName, "GetStartupInfoW"))
		{
			return GetStartupInfoWHook;
		}
		else if (!_stricmp(functionName, "SetWindowsHookExA"))
		{
			return NoWindowsHookExA;
		}
		else if (!_stricmp(functionName, "CreateFileW"))
		{
			return CreateFileWHook;
		}
		else if (!_stricmp(functionName, "GetFileAttributesExW"))
		{
			return GetFileAttributesExWHook;
		}
		else if (!_stricmp(functionName, "GetFileAttributesW"))
		{
			return GetFileAttributesWHook;
		}
#endif

		return (LPVOID)GetProcAddress(module, functionName);
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

#if !defined(PAYNE) && !defined(GTA_FIVE)
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

	{
		// user library stuff ('safe' ntdll hooking callbacks)
		wchar_t ntdllPath[MAX_PATH];
		GetModuleFileName(GetModuleHandle(L"ntdll.dll"), ntdllPath, _countof(ntdllPath));

		NtdllHooks hooks(ntdllPath);
		hooks.Install();
	}

	if (CoreIsDebuggerPresent())
	{
		// NOP OutputDebugStringA; the debugger doesn't like multiple async exceptions
		uint8_t* func = (uint8_t*)OutputDebugStringA;

		DWORD oldProtect;
		VirtualProtect(func, 1, PAGE_EXECUTE_READWRITE, &oldProtect);

		//*func = 0xC3;

		VirtualProtect(func, 1, oldProtect, &oldProtect);
	}

	g_launcher = launcher;
#endif

	AddVectoredExceptionHandler(0, HandleVariant);

#ifndef _M_AMD64
	return InvokeEntryPoint(entryPoint);
#else
	return entryPoint();
#endif
}