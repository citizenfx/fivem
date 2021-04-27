#pragma once

#include <WS2tcpip.h>

namespace rage
{
	struct rlGamerInfo;
}

#ifdef COMPILING_GTA_GAME_NY
#define GAME_EXPORT DLL_EXPORT
#else
#define GAME_EXPORT DLL_IMPORT
#endif

class GAME_EXPORT CPlayerInfo
{
private:
	char pad[1432];
	void* ped;

public:
	static CPlayerInfo* GetLocalPlayer();

	static CPlayerInfo* GetPlayer(int index);

	inline void* GetPed() 
	{
		return ped;
	}

	inline rage::rlGamerInfo* GetGamerInfo()
	{
		return (rage::rlGamerInfo*)this;
	}
};
