#include "StdInc.h"
#include "CitizenGame.h"

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

void InitializeDummies();

void main()
{
	// initialize the CRT
	_CRT_INIT(GetModuleHandle(NULL), DLL_PROCESS_ATTACH, NULL);

	// TODO: bootstrap updater; downloading new executable (if old deployment plan is actually in-use)
	
	// path environment appending of our primary directories
	static wchar_t pathBuf[32768];
	GetEnvironmentVariable(L"PATH", pathBuf, sizeof(pathBuf));

	std::wstring newPath = MakeRelativeCitPath(L"citmp\\bin") + L";" + MakeRelativeCitPath(L"plaza") + L";" + MakeRelativeGamePath(L"") + L";" + std::wstring(pathBuf);

	SetEnvironmentVariable(L"PATH", newPath.c_str());

	SetDllDirectory(MakeRelativeGamePath(L"").c_str()); // to prevent a) current directory DLL search being disabled and b) xlive.dll being taken from system if not overridden
	SetCurrentDirectory(MakeRelativeGamePath(L"").c_str());

	// initialize TLS variable so we get a TLS directory
	InitializeDummies();

	// check stuff regarding the game executable
	std::wstring gameExecutable = MakeRelativeGamePath(L"GTAIV.exe");

	if (GetFileAttributes(gameExecutable.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		ExitProcess(1);
	}

	// main updater stuff

	// game launcher initialization
	CitizenGame::Launch(gameExecutable);
}