#include "StdInc.h"
#include "CPlayerInfo.h"

int* localPlayer;

static hook::cdecl_stub<CPlayerInfo*(int)> _getPlayer([]()
{
	return hook::get_pattern("8b 44 24 04 83 f8 1f 77 08");
});

CPlayerInfo* CPlayerInfo::GetPlayer(int index)
{
	return _getPlayer(index);
}

CPlayerInfo* CPlayerInfo::GetLocalPlayer()
{
	if (*localPlayer != -1)
	{
		return GetPlayer(*localPlayer);
	}

	return nullptr;
}

static HookFunction hookFunc([]()
{
	localPlayer = *hook::get_pattern<int*>("40 83 C6 7C 83 C4 04", 51);
});
