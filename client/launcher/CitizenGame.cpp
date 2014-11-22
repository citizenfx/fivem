#include "StdInc.h"
#include "CitizenGame.h"
#include "ExecutableLoader.h"
#include "LauncherInterface.h"

#include "include/cef_sandbox_win.h"

#include "Hooking.h"

#include <winternl.h>

#if defined(PAYNE)
BYTE g_gmfOrig[5];
BYTE g_gmfOrigW[5];
BYTE g_gsiOrig[5];

ILauncherInterface* g_launcher;

VOID WINAPI GetStartupInfoAHook(LPSTARTUPINFOA startupInfo)
{
	// put back the original and call it
	memcpy(GetStartupInfoA, g_gsiOrig, 5);

	GetStartupInfoA(startupInfo);

	// analyze the return address for a recommended course of action (GameShield is using a Borland-style compiler, so the only VC CRT here will be Payne)
	char* charData = (char*)_ReturnAddress();

	if (!memcmp(charData, "\x6A\xFE\x5F\x89\x7D\xFC\xB8\x4D", 8))
	{
		if (!g_launcher->PostLoadGame(GetModuleHandle(nullptr), nullptr))
		{
			ExitProcess(0);
		}
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

	// and call the entry point
	entryPoint();
}