#include "StdInc.h"

struct CEpisode
{
	char path[260];
	char pad[64];
	char device[20]; // +324
	int deviceType; // +344
	char pad2[8];
	char pad3[2];
	char useThis; // +358; checked before calling the 'scan and process episode setup' func
	char unknown1; // +359
};

struct CEpisodes
{
	char pad[332];
	CEpisode* episodes; // +332
	short numEpisodes; // +336
	short numAllocatedEpisodes; // +338
	char pad2[18]; // +340
	char unknownScanFlag; // +358

	void addEpisode(const CEpisode* episode);
};

void WRAPPER CEpisodes::addEpisode(const CEpisode* episode) { EAXJMP(0x813E20); }

// process a CEpisode, checking activation and some other stuff
auto ProcessEpisode = (void(__stdcall*)(int episodeMask, CEpisode* episode, int unknown1))0x813480;

void AddEpisode(CEpisodes* episodes, int num, const char* path)
{
	CEpisode ep;
	memset(&ep, 0, sizeof(ep));
	strcpy(ep.path, path);
	ep.useThis = 1;
	ep.deviceType = 1;

	ProcessEpisode(num, &ep, 1);

	episodes->addEpisode(&ep);
}

void __fastcall ScanGameEpisodes(CEpisodes* episodes)
{
	std::wstring citizenPath = MakeRelativeCitPath(L"citizen");
	char citizenPathStr[256];

	wcstombs(citizenPathStr, citizenPath.c_str(), sizeof(citizenPathStr));

	AddEpisode(episodes, 4, citizenPathStr);

	HKEY regKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Rockstar Games\\EFLC", 0, KEY_READ, &regKey) == ERROR_SUCCESS)
	{
		char installFolder[512];
		DWORD numBytes = sizeof(installFolder);

		if (RegQueryValueExA(regKey, "InstallFolder", 0, nullptr, (LPBYTE)installFolder, &numBytes) == ERROR_SUCCESS)
		{
			if (GetFileAttributesA(installFolder) != INVALID_FILE_ATTRIBUTES)
			{
				char pathStr[256];
				strcpy(pathStr, installFolder);
				strcat(pathStr, "\\TLAD\\");

				AddEpisode(episodes, 2, pathStr);

				strcpy(pathStr, installFolder);
				strcat(pathStr, "\\TBoGT\\");

				AddEpisode(episodes, 8, pathStr);
			}
		}
	}

	episodes->unknownScanFlag = 2;
}

static DWORD EnableEpisode = 0x8147A0;

void __declspec(naked) GameInitEpisodeHack()
{
	__asm
	{
		mov esi, dword ptr ds:[10F47F4h]

		mov ecx, dword ptr ds:[19AB8F0h]
		push 1
		push 2 // episode num

		call EnableEpisode

		mov eax, 2
		retn
	}
}

void __declspec(naked) IgnoreNorthEpisodesHook()
{
	__asm
	{
		mov eax, [esp + 4h]
		mov al, byte ptr [eax + 228h]

		cmp al, 2
		jz returnNormally

		cmp al, 4
		jz returnNormally

		push 0813D50h
		retn

returnNormally:
		retn 4
	}
}

static HookFunction hookFunction([] ()
{
	// episode scanning function
	hook::jump(0x813F30, ScanGameEpisodes);
	
	// supporting patches

	// skip SecuROM activation error check and mov eax, 1
	hook::put<uint8_t>(0x813161, 0xB8);
	hook::put<uint32_t>(0x813162, 1);

	// don't go to an error scenario if XLiveInstall fails
	hook::nop(0x813138, 2);

	// as above, size fail check
	hook::nop(0x81314C, 2);

	// don't even try activating SecuROM
	hook::nop(0x413159, 5);

	// ignore R*N episodes
	hook::call(0x814E1D, IgnoreNorthEpisodesHook);

	// always load episode 2
	hook::nop(0x4205B3, 6);
	hook::call(0x4205B4, GameInitEpisodeHack);

	// don't fuck loading screens, you dummy
	hook::nop(0x420783, 5); // unload
	hook::nop(0x4207CB, 5); // reload

	// ignore unknown check regarding loading type causing scripts to not process
	// don't mind that, that was just hookin' stupidity
	//hook::nop(0x42024F, 2);

	// ignore weird episode shit
	hook::put<uint8_t>(0x42072C, 0xEB);
	hook::put<uint8_t>(0x420780, 0xEB);

	// or these for the above
	hook::nop(0x420652, 6);
	hook::nop(0x42065C, 6);
});