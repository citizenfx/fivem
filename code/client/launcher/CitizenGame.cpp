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

#include <HostSharedData.h>
#include <CfxState.h>
#include <UUIState.h>

#include <Error.h>

static ILauncherInterface* g_launcher;

void InitializeMiniDumpOverride();

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

static void* DeleteVideo(void*, char* videoName)
{
	DWORD oldProtect;
	VirtualProtect(videoName, 4, PAGE_READWRITE, &oldProtect);
	strcpy(videoName, "nah");
	return nullptr;
}

static void InitialGameHook()
{
	// ignore launcher requirement
	// 1737 updated this for MTL
	// and 1868 made it Arxan.
	hook::call(hook::pattern("84 C0 75 0C B2 01 B9 2F A9 C2 F4").count(1).get(0).get<void>(-5), ThisIsActuallyLaunchery);

	// ignore steam requirement
	/*auto pattern = hook::pattern("FF 15 ? ? ? ? 84 C0 74 0C B2 01 B9 91 32 25");// 31 E8");
	if (pattern.size() > 0)
	{
		hook::nop(pattern.get(0).get<void>(0), 6);
		hook::put<uint8_t>(pattern.get(0).get<void>(8), 0xEB);
	}*/

	// ignore loading 'videos'
	hook::call(hook::get_pattern("8B F8 48 85 C0 74 47 48 8B C8 E8 ? ? ? ? 4C", -6), DeleteVideo);
	
	// draw loading screen even if 'not' enabled
	hook::nop(hook::get_pattern("85 DB 0F 84 ? ? ? ? 8B 05 ? ? ? ? A8 01 75 15 83", 2), 6);
}
#elif defined(IS_RDR3)
static void* g_f;
static char g_b[5];

int ThisIsActuallyLaunchery()
{
	memcpy(g_f, g_b, 5);

	return 1;
}

static void InitialGameHook()
{
	g_f = hook::get_call(hook::get_pattern("BA 99 CA D1 D1 B9 2F A9 C2 F4", -28));
	memcpy(g_b, g_f, 5);
	hook::jump(g_f, ThisIsActuallyLaunchery);
}
#else
static void InitialGameHook()
{
}
#endif

bool g_ranStartupInfo;

VOID WINAPI GetStartupInfoWHook(_Out_ LPSTARTUPINFOW lpStartupInfo)
{
	GetStartupInfoW(lpStartupInfo);

	if (g_ranStartupInfo)
	{
		return;
	}

	g_ranStartupInfo = true;

	hook::set_base();

	if (getenv("CitizenFX_ToolMode"))
	{
		auto plRoutine = (void (*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "ToolMode_RunPostLaunchRoutine");
		plRoutine();

		return;
	}

	InitialGameHook();

	if (!g_launcher->PostLoadGame(GetModuleHandle(nullptr), nullptr))
	{
		ExitProcess(0);
	}

	if (!g_launcher->PreResumeGame())
	{
		ExitProcess(0);
	}
}

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

#if defined(GTA_FIVE) || defined(IS_RDR3) || defined(GTA_NY)
static int WINAPI NoWindowsHookExA(int, HOOKPROC, HINSTANCE, DWORD)
{
	return 1;
}

extern std::map<std::string, std::string> g_redirectionData;

static std::wstring MapRedirectedFilename(const wchar_t* lpFileName)
{
	// original filename with backslashes converted to slashes
	std::wstring origFileName = lpFileName;
	std::replace(origFileName.begin(), origFileName.end(), L'\\', L'/');

	std::string fileName = ToNarrow(origFileName);

	for (auto& redirectedPair : g_redirectionData)
	{
		if (fileName.length() >= redirectedPair.first.length())
		{
			size_t start = fileName.length() - redirectedPair.first.length();

			// FIXME: what if this is actually a UTF-8 subfilename?
			if (_stricmp(fileName.substr(start).c_str(), redirectedPair.first.c_str()) == 0)
			{
				fileName = redirectedPair.second;

				origFileName = ToWide(fileName);

				return origFileName;
			}
		}
	}

	return lpFileName;
}

static HANDLE WINAPI CreateFileWHook(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return CreateFileW(fileName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static DWORD WINAPI GetFileAttributesWHook(_In_ LPCWSTR lpFileName)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return GetFileAttributesW(fileName.c_str());
}

static BOOL WINAPI GetFileAttributesExWHook(_In_ LPCWSTR lpFileName, _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId, _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return GetFileAttributesExW(fileName.c_str(), fInfoLevelId, lpFileInformation);
}
#else
static std::wstring MapRedirectedFilename(const wchar_t* lpFileName)
{
	return lpFileName;
}
#endif

void CitizenGame::SetCoreMapping()
{
#ifndef IS_LAUNCHER
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
#endif
}

#include <sddl.h>
#pragma comment(lib, "ntdll.lib")

void AAD_Initialize()
{
#if defined(GTA_FIVE) || defined(IS_RDR3)
	// set BeingDebugged
	PPEB peb = (PPEB)__readgsqword(0x60);
	peb->BeingDebugged = false;

	// set GlobalFlags
	*(DWORD*)((char*)peb + 0xBC) &= ~0x70;
#elif defined(GTA_NY)
	// set BeingDebugged
	PPEB peb = (PPEB)__readfsdword(0x30);
	peb->BeingDebugged = false;

	// set GlobalFlags
	*(DWORD*)((char*)peb + 0x68) &= ~0x70;
#endif

	AddVectoredExceptionHandler(0, HandleVariant);
}

#include <psapi.h>

BOOL EnumProcessModulesHook(
	HANDLE  hProcess,
	HMODULE* lphModule,
	DWORD   cb,
	LPDWORD lpcbNeeded
)
{
	trace("enum modules\n");

	return EnumProcessModules(hProcess, lphModule, cb, lpcbNeeded);
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

static std::string g_minModeManifest;

void CitizenGame::SetMinModeManifest(const std::string& manifest)
{
	g_minModeManifest = manifest;
}

inline void CoreSetMinModeManifest(const char* str)
{
	static void(*func)(const char*);

	if (!func)
	{
		func = (void(*)(const char*))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreSetMinModeManifest");
	}

	(func) ? func(str) : (void)0;
}

extern void DLLError(DWORD errorCode, std::string_view dllName);

void CitizenGame::Launch(const std::wstring& gamePath, bool isMainGame)
{
	// initialize the CEF sandbox
	void* sandboxInfo = nullptr;

	// load the game library
	HMODULE gameLibrary = LoadLibrary(MakeRelativeCitPath(L"CitizenGame.dll").c_str());

	if (!gameLibrary)
	{
		DWORD errorCode = GetLastError();
		DLLError(errorCode, "CitizenGame.dll");
		return;
	}

#ifdef GTA_FIVE
	wchar_t moduleName[MAX_PATH];
	GetModuleFileNameW(GetModuleHandleW(NULL), moduleName, std::size(moduleName));

	// unless we're ChromeBrowser or ROSLauncher, try loading scripthookv.dll
	if (wcsstr(moduleName, L"ChromeBrowser") == nullptr && wcsstr(moduleName, L"ROSLauncher") == nullptr)
	{
		// force loading _this_ variant
		LoadLibrary(MakeRelativeCitPath(L"scripthookv.dll").c_str());
	}
#endif

	CoreSetDebuggerPresent();
	CoreSetMinModeManifest(g_minModeManifest.c_str());

	if (!getenv("CitizenFX_ToolMode"))
	{
		SetCoreMapping();
	}

	InitializeMiniDumpOverride();

	// get the launcher interface
	GetLauncherInterface_t getLauncherInterface = (GetLauncherInterface_t)GetProcAddress(gameLibrary, "GetLauncherInterface");

	if (!getLauncherInterface)
	{
		return;
	}

#ifdef GTA_NY
	AAD_Initialize();
#endif

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

	static HostSharedData<CfxState> initState("CfxInitState");

	// prevent accidental duplicate instances
	if (isMainGame && !initState->IsMasterProcess() && !initState->IsGameProcess())
	{
		return;
	}

#ifdef LAUNCHER_PERSONALITY_GAME
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
	HMODULE exeModule = (HMODULE)&__ImageBase;

	ExecutableLoader exeLoader(data);
#if defined(GTA_NY)
	exeLoader.SetLoadLimit(0x1BE8000 + 0x400000);
#elif defined(PAYNE)
	exeLoader.SetLoadLimit(0x20000000);
#elif defined(GTA_FIVE)
	exeLoader.SetLoadLimit(0x140000000 + 0x60000000);
#elif defined(IS_RDR3)
	exeLoader.SetLoadLimit(0x140000000 + 0x80000000);
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
			std::wstring gameDll = MakeRelativeCitPath(L"bin\\SwiftShaderD3D9_64.dll");

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
				// pre-load the correct chrome_elf.dll
				LoadLibraryW(MapRedirectedFilename(L"Social Club/chrome_elf.dll").c_str());

				return LoadLibraryW(MapRedirectedFilename(L"Social Club/libcef.dll").c_str());
			}
		}

		return LoadLibraryA(libName);
	});

	exeLoader.SetFunctionResolver([] (HMODULE module, const char* functionName) -> LPVOID
	{
		if (!_stricmp(functionName, "GetStartupInfoW"))
		{
			return GetStartupInfoWHook;
		}

#if defined(GTA_FIVE) || defined(IS_RDR3)
		if (!_stricmp(functionName, "SetWindowsHookExA"))
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
		else if (!_stricmp(functionName, "K32EnumProcessModules"))
		{
			return EnumProcessModulesHook;
		}
#endif

		return (LPVOID)GetProcAddress(module, functionName);
	});

	exeLoader.LoadIntoModule(exeModule);

#if defined(GTA_FIVE) && defined(RWX_TEST)
	if (!getenv("CitizenFX_ToolMode"))
	{
		exeLoader.Protect();
	}
#endif

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

	// i.e. not using GetStartupInfo hook
#if !defined(PAYNE) && !defined(GTA_FIVE) && !defined(IS_RDR3) && !defined(GTA_NY)
	if (!launcher->PostLoadGame(exeModule, &entryPoint))
	{
		ExitProcess(0);
	}

	if (!launcher->PreResumeGame())
	{
		ExitProcess(0);
	}
#endif

	//
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

#endif

	// store the launcher interface
	g_launcher = launcher;

#ifndef GTA_NY
	AAD_Initialize();
#endif

#ifndef _M_AMD64
	return InvokeEntryPoint(entryPoint);
#else
	return entryPoint();
#endif
#else
	return;
#endif
}
