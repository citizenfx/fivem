#include "StdInc.h"
#include "ExecutableLoader.h"

#include "Hooking.h"

#include <shellapi.h>

static VOID WINAPI ExecutablePreload_GetStartupInfoW(_Out_ LPSTARTUPINFOW lpStartupInfo)
{
	GetStartupInfoW(lpStartupInfo);

	// as this is only used in case of --find_ep, save the EP
	FILE* f = _wfopen(MakeRelativeCitPath(L"cache/found_ep.bin").c_str(), L"w");

	if (!f)
	{
		TerminateProcess(GetCurrentProcess(), 0);
	}

	char* module = (char*)GetModuleHandle(nullptr);
	IMAGE_DOS_HEADER* header = (IMAGE_DOS_HEADER*)module;
	IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(module + header->e_lfanew);

	fwrite(&ntHeader->OptionalHeader.AddressOfEntryPoint, 1, sizeof(DWORD), f);
	fclose(f);

	TerminateProcess(GetCurrentProcess(), 4343);
}

static bool LoadExecutable(const wchar_t* executablePath, void** entryPointOut = nullptr)
{
	FILE* gameFile = _wfopen(executablePath, L"rb");

	if (!gameFile)
	{
		return false;
	}

	// find the file length and allocate a related buffer
	uint32_t length;

	fseek(gameFile, 0, SEEK_END);
	length = ftell(gameFile);

	std::vector<uint8_t> data(length);

	// seek back to the start and read the file
	fseek(gameFile, 0, SEEK_SET);
	fread(data.data(), 1, length, gameFile);

	// close the file, and continue on
	fclose(gameFile);
	
	// load the executable
	ExecutableLoader exeLoader(data.data());
	exeLoader.SetLoadLimit(0x140000000 + 0x60000000);
	exeLoader.SetLibraryLoader([](const char* libName)
	{
		auto hMod = LoadLibraryA(libName);

		if (hMod == nullptr)
		{
			hMod = (HMODULE)INVALID_HANDLE_VALUE;
		}

		return hMod;
	});

	exeLoader.SetFunctionResolver([](HMODULE module, const char* functionName)->LPVOID
	{
		if (!_stricmp(functionName, "GetStartupInfoW"))
		{
			return ExecutablePreload_GetStartupInfoW;
		}

		return GetProcAddress(module, functionName);
	});

	exeLoader.LoadIntoModule(GetModuleHandle(nullptr));

	if (entryPointOut)
	{
		*entryPointOut = exeLoader.GetEntryPoint();
	}
}

void AAD_Initialize();

void DoCreateDump(void* ep, const wchar_t* fileName);

static void DumpExecutable(const wchar_t* executablePath, const wchar_t* outPath, uintptr_t epOffset)
{
	void* fakeEP;

	if (LoadExecutable(executablePath, &fakeEP))
	{
		AAD_Initialize();
		DoCreateDump((void*)((char*)GetModuleHandle(nullptr) + epOffset + 9), outPath);

		((void(*)())fakeEP)();
	}
}

static void FindEP(const wchar_t* executablePath)
{
	void* fakeEP;

	if (LoadExecutable(executablePath, &fakeEP))
	{
		AAD_Initialize();
		((void(*)())fakeEP)();
	}
}

static bool ExecutablePreload_HandleArgs()
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

	if (argc == 5 && !_wcsicmp(argv[1], L"-dump_exe"))
	{
		DumpExecutable(argv[2], argv[3], wcstoull(argv[4], nullptr, 10));

		LocalFree(argv);
		return false;
	}
	else if (argc == 3 && !_wcsicmp(argv[1], L"-find_ep"))
	{
		FindEP(argv[2]);

		LocalFree(argv);
		return false;
	}

	LocalFree(argv);

	return true;
}

bool ExecutablePreload_Init()
{
	if (!ExecutablePreload_HandleArgs())
	{
		return false;
	}

	

	return true;
}
