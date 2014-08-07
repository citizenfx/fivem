#include "StdInc.h"
#include "CitizenGame.h"

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

void InitializeDummies();
void EnsureGamePath();

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo);

void main()
{
	// initialize the CRT
	_CRT_INIT(GetModuleHandle(NULL), DLL_PROCESS_ATTACH, NULL);

	// bootstrap the game
	if (Bootstrap_RunInit())
	{
		ExitProcess(0);
	}

	//SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

	EnsureGamePath();
	
	// path environment appending of our primary directories
	static wchar_t pathBuf[32768];
	GetEnvironmentVariable(L"PATH", pathBuf, sizeof(pathBuf));

	std::wstring newPath = MakeRelativeCitPath(L"bin") + L";" + MakeRelativeCitPath(L"plaza") + L";" + MakeRelativeGamePath(L"") + L";" + std::wstring(pathBuf);

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