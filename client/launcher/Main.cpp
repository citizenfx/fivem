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

#pragma comment(lib, "version.lib")

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

#ifdef GTA_FIVE
	{
		DWORD versionInfoSize = GetFileVersionInfoSize(gameExecutable.c_str(), nullptr);

		if (versionInfoSize)
		{
			std::vector<uint8_t> versionInfo(versionInfoSize);

			if (GetFileVersionInfo(gameExecutable.c_str(), 0, versionInfo.size(), &versionInfo[0]))
			{
				void* fixedInfoBuffer;
				UINT fixedInfoSize;

				VerQueryValue(&versionInfo[0], L"\\", &fixedInfoBuffer, &fixedInfoSize);

				VS_FIXEDFILEINFO* fixedInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(fixedInfoBuffer);
				
				if ((fixedInfo->dwFileVersionLS >> 16) != 350)
				{
					MessageBox(nullptr, va(L"The found GTA executable (%s) has version %d.%d.%d.%d, but only 1.0.350 is currently supported. Please obtain this version, and try again.",
										   gameExecutable.c_str(),
										   (fixedInfo->dwFileVersionMS >> 16),
										   (fixedInfo->dwFileVersionMS & 0xFFFF),
										   (fixedInfo->dwFileVersionLS >> 16),
										   (fixedInfo->dwFileVersionLS & 0xFFFF)), PRODUCT_NAME, MB_OK | MB_ICONERROR);

					ExitProcess(1);
				}
			}
		}
	}
#endif

	// main updater stuff

	// game launcher initialization
	CitizenGame::Launch(gameExecutable);
}