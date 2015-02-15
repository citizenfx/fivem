/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include "NativeEpisode.h"

int GetCurrentEpisodeID();

int g_useNorthEpisodes;

CEpisodes*& g_episodes = *(CEpisodes**)0x19AB8F0;

void WRAPPER CEpisodes::addEpisode(const CEpisode* episode) { EAXJMP(0x813E20); }

void WRAPPER CEpisodes::ScanEpisodes()
{
	EAXJMP(0x813BD0);
}

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

		call GetCurrentEpisodeID

		push eax

		mov ecx, dword ptr ds:[19AB8F0h]
		push 1
		push eax // episode num

		call EnableEpisode

		pop eax

		//mov eax, 2
		retn
	}
}

void __declspec(naked) IgnoreNorthEpisodesHook()
{
	__asm
	{
		mov eax, g_useNorthEpisodes
		test eax, eax

		jnz executeEpisode

		mov eax, [esp + 4h]
		mov al, byte ptr [eax + 228h]

		cmp al, 2
		jz returnNormally

		cmp al, 4
		jz returnNormally

executeEpisode:
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
	// actually not, this is important
	hook::nop(0x420652, 6); // <- ignore episode changes as reason to reload the base game
	//hook::nop(0x42065C, 6);

	// workaround for episode reloading causing device/ipl tables to contain 0 devices for sources actually being different devices (platformimg:/ and e2:/ - e2-e0-e1-e2 load order is a repro)
	// this makes the function ignore devices being the same for reloading, as >why would someone load the same IPL twice during the same loading sequence anyway
	hook::nop(0x8D8843, 2);
	hook::nop(0x8D8963, 2); // delayed ipl?
});