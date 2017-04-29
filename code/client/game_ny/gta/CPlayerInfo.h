#pragma once

#include <WS2tcpip.h>

typedef struct
{
	IN_ADDR ina;
	IN_ADDR inaOnline;
	WORD wPortOnline;
	BYTE abEnet[6];
	BYTE abOnline[20];
} XNADDR;

class GAMESPEC_EXPORT CPlayerInfo
{
public:
	XNADDR address; // 36 bytes long
	char pad1[4];
	uint32_t netSourceID1;
	uint32_t netSourceID2;
	char pad[1372];
	void* ped;

	static inline CPlayerInfo* GetLocalPlayer()
	{
		int localPlayer = *(int*)0xF1CC68;

		if (localPlayer != -1)
		{
			return GetPlayer(localPlayer);
		}

		return nullptr;
	}

	static CPlayerInfo* GetPlayer(int index);
};