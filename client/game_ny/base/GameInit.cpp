#include "StdInc.h"
#include "GameInit.h"
#include "Hooking.h"

int hook::baseAddressDifference;

bool GameInit::GetGameLoaded()
{
	return *(uint8_t*)0xF22B3C;
}

void GameInit::LoadGameFirstLaunch()
{
	assert(!GameInit::GetGameLoaded());

	//*(DWORD*)0x10C7F80 = 6;
}

void GameInit::ReloadGame()
{ }

static InitFunction initFunction([] ()
{
	// unused byte which is set to 0 during loading
	hook::put<uint8_t>(0xF22B3C, 1);
});