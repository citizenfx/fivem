#include "StdInc.h"
#include "CitizenGame.h"
#include "ExecutableLoader.h"
#include "LauncherInterface.h"

void CitizenGame::Launch(std::wstring& gamePath)
{
	// load the game library
	HMODULE gameLibrary = LoadLibrary(MakeRelativeCitPath(L"CitizenGame.dll").c_str());

	if (!gameLibrary)
	{
		// TODO: report error
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
	if (!launcher->PreLoadGame())
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
	exeLoader.SetLoadLimit(0xF0D000);
	exeLoader.SetLibraryLoader([] (const char* libName)
	{
		if (!_stricmp(libName, "xlive.dll"))
		{
			return (HMODULE)INVALID_HANDLE_VALUE;
		}

		return LoadLibraryA(libName);
	});

	exeLoader.LoadIntoModule(exeModule);
	
	// free the old binary
	delete[] data;

	// call into the launcher interface
	void(*entryPoint)();

	if (!launcher->PostLoadGame(exeModule, &entryPoint))
	{
		ExitProcess(0);
	}

	// and call the entry point
	entryPoint();
}