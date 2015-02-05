/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CitizenGame.h"

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

void InitializeDummies();
void EnsureGamePath();

bool InitializeExceptionHandler();

void main()
{
	// initialize the CRT
	_CRT_INIT(GetModuleHandle(NULL), DLL_PROCESS_ATTACH, NULL);

	// bootstrap the game
	if (Bootstrap_RunInit())
	{
		ExitProcess(0);
	}

	if (InitializeExceptionHandler())
	{
		ExitProcess(0);
	}

	// path environment appending of our primary directories
	static wchar_t pathBuf[32768];
	GetEnvironmentVariable(L"PATH", pathBuf, sizeof(pathBuf));

	std::wstring newPath = MakeRelativeCitPath(L"bin") + L";" + std::wstring(pathBuf);

	SetEnvironmentVariable(L"PATH", newPath.c_str());

	SetDllDirectory(MakeRelativeCitPath(L"bin").c_str()); // to prevent a) current directory DLL search being disabled and b) xlive.dll being taken from system if not overridden

	SetCurrentDirectory(MakeRelativeCitPath(L"").c_str());

	// determine dev mode and do updating
	wchar_t exeName[512];
	GetModuleFileName(GetModuleHandle(NULL), exeName, sizeof(exeName) / 2);

	wchar_t* exeBaseName = wcsrchr(exeName, L'\\');
	exeBaseName[0] = L'\0';
	exeBaseName++;

	bool devMode = false;

	if (GetFileAttributes(va(L"%s.dev", exeBaseName)) != INVALID_FILE_ATTRIBUTES)
	{
		devMode = true;
	}

	if (!devMode)
	{
		if (!Bootstrap_DoBootstrap())
		{
			ExitProcess(0);
			return;
		}
	}

	EnsureGamePath();

	// readd the game path into the PATH
	newPath = MakeRelativeCitPath(L"bin") + L";" + MakeRelativeGamePath(L"") + L";" + std::wstring(pathBuf);

	SetEnvironmentVariable(L"PATH", newPath.c_str());

	SetCurrentDirectory(MakeRelativeGamePath(L"").c_str());

#ifdef GTA_NY
	// initialize TLS variable so we get a TLS directory
	InitializeDummies();
#endif

	// check stuff regarding the game executable
	std::wstring gameExecutable = MakeRelativeGamePath(GAME_EXECUTABLE);

	if (GetFileAttributes(gameExecutable.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		MessageBox(nullptr, L"Could not find the game executable (" GAME_EXECUTABLE L") at the configured path. Please check your CitizenFX.ini file.", PRODUCT_NAME, MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}

	// main updater stuff

	// game launcher initialization
	CitizenGame::Launch(gameExecutable);
}