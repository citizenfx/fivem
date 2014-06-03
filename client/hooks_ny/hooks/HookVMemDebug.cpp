#include "StdInc.h"

char __fastcall DoWantHudZone(char* stuff)
{
	DWORD texRamUsed = ((DWORD(*)())0x624770)();

	swprintf((wchar_t*)(stuff + 134), L"%d/%d MB V~n~%d/%d MB S", texRamUsed / 1024 / 1024, *(DWORD*)(0x18A867C) / 1024 / 1024, *(DWORD*)(0xF21C84) / 1024 / 1024, *(DWORD*)(0xF21C80) / 1024 / 1024);

	return true;
}

static HookFunction hookFunction([] ()
{
	//hook::call(0x427FBF, DoWantHudZone);
});