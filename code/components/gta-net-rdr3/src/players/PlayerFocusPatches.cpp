#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <MinHook.h>

#include <DirectXMath.h>
#include <GameInit.h>

#include <netPlayerManager.h>
#include <netBitsets.h>

#include <PlayerLimits.h>

#include "PlayerPatches.h"

namespace rage
{
	using Vec3V = DirectX::XMVECTOR;
}

using rage::kMaxPlayers;

extern ICoreGameInit* icgi;
extern CNetGamePlayer* g_players[256];
extern CNetGamePlayer* g_playerListRemote[256];
extern int g_playerListCountRemote;

extern rage::atPlayerBitsets g_playerFocusPositionUpdateBitset;

alignas(16) static rage::Vec3V g_playerFocusPositions[kMaxPlayers + 1];
static uint32_t g_playerInteriorHandles[kMaxPlayers + 1] = {};

static hook::cdecl_stub<rage::Vec3V*(rage::Vec3V*, CNetGamePlayer*, char*)> _getNetPlayerFocusPosition([]()
{
	return hook::get_pattern("41 22 C2 41 3A C1 0F 93 C0", -0x74);
});

static hook::cdecl_stub<void*(CNetGamePlayer*)> getPlayerPedForNetPlayer([]()
{
	return hook::get_call(hook::get_pattern("48 8B CD 0F 11 06 48 8B D8 E8", -8));
});

static void (*g_origUpdatePlayerFocusPosition)(void*, CNetGamePlayer*);
static DirectX::XMVECTOR* (*g_origGetNetPlayerRelevancePosition)(DirectX::XMVECTOR* position, CNetGamePlayer* player, void* unk);

namespace rage
{
void UpdatePlayerFocusPosition(void* objMgr, CNetGamePlayer* player)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origUpdatePlayerFocusPosition(objMgr, player);
	}

	uint8_t playerIndex = player->physicalPlayerIndex();
	bool isRemote = playerIndex == 31;

	// Find the players index if remote.
	if (isRemote)
	{
		for (int i = 0; i < 256; i++)
		{
			if (g_players[i] == player)
			{
				playerIndex = i;
				break;
			}
		}
	}

	if (playerIndex > kMaxPlayers)
	{
		return;
	}

	DirectX::XMVECTOR position;
	char outFlag = 0;
	DirectX::XMVECTOR* outPosition = _getNetPlayerFocusPosition(&position, player, &outFlag);

	g_playerFocusPositions[playerIndex] = *outPosition;
	if (outFlag != 0)
	{
		g_playerFocusPositionUpdateBitset.SetValue(playerIndex);
	}
}

DirectX::XMVECTOR* GetPlayerFocusPosition(DirectX::XMVECTOR* position, CNetGamePlayer* player, uint8_t* unk)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetNetPlayerRelevancePosition(position, player, unk);
	}

	bool isRemote = player->physicalPlayerIndex() == 31;
	uint8_t playerIndex = player->physicalPlayerIndex();

	// Find the players index if remote.
	if (isRemote)
	{
		for (int i = 0; i < 256; i++)
		{
			if (g_players[i] == player)
			{
				playerIndex = i;
				break;
			}
		}
	}

	if (playerIndex > kMaxPlayers)
	{
		return nullptr;
	}

	if (unk)
	{
		*unk = g_playerFocusPositionUpdateBitset.IsSet(playerIndex);
	}

	*position = g_playerFocusPositions[playerIndex];
	return position;
}
}

static float VectorDistance(rage::Vec3V a, rage::Vec3V b)
{
	rage::Vec3V delta = DirectX::XMVectorSubtract(a, b);
	float dis = DirectX::XMVectorGetX(DirectX::XMVector3Length(delta));
	return dis;
}

static int (*g_origGetPlayersNearPoint)(rage::Vec3V* point, uint32_t unkIndex, void* outIndex, CNetGamePlayer* outArray[32], bool unkVal, float range, bool sorted);
static int GetPlayersNearPoint(rage::Vec3V* point, uint32_t unkIndex, void* outIndex, CNetGamePlayer* outArray[32], bool unkVal, float range, bool sorted)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPlayersNearPoint(point, unkIndex, outIndex, outArray, range, range, sorted);
	}

	CNetGamePlayer* tempArray[kMaxPlayers + 1]{};

	int idx = 0;

	auto playerList = g_playerListRemote;
	for (int i = 0; i < g_playerListCountRemote; i++)
	{
		auto player = playerList[i];

		if (player && getPlayerPedForNetPlayer(player))
		{
			rage::Vec3V vectorPos;

			if (range >= 100000000.0f || VectorDistance(*point, *rage::GetPlayerFocusPosition(&vectorPos, player, nullptr)) < range)
			{
				tempArray[idx] = player;
				idx++;
			}
		}
	}

	if (sorted)
	{
		std::sort(tempArray, tempArray + idx, [point](CNetGamePlayer* a1, CNetGamePlayer* a2)
		{
			rage::Vec3V vectorPos1;
			rage::Vec3V vectorPos2;

			float d1 = VectorDistance(*point, *rage::GetPlayerFocusPosition(&vectorPos1, a1, nullptr));
			float d2 = VectorDistance(*point, *rage::GetPlayerFocusPosition(&vectorPos2, a2, nullptr));

			return (d1 < d2);
		});
	}

	idx = std::min(idx, 32);
	unkIndex = idx;
	std::copy(tempArray, tempArray + idx, outArray);

	return idx;
}

static void (*g_origNetworkObjectMgr__UpdateBitsets)(void*);
static void CNetworkObjectMgr__UpdateBitsets(void* networkMgr)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origNetworkObjectMgr__UpdateBitsets(networkMgr);
	}

	g_playerFocusPositionUpdateBitset.Clear();

	for (auto& player : g_players)
	{
		if (!player)
		{
			continue;
		}

		// also updates player focus bitset.
		rage::UpdatePlayerFocusPosition(networkMgr, player);

		hook::FlexStruct* playerPed = reinterpret_cast<hook::FlexStruct*>(getPlayerPedForNetPlayer(player));
		if (!playerPed)
		{
			continue;
		}

		hook::FlexStruct* netObj = playerPed->At<hook::FlexStruct*>(0xE0);
		if (!netObj)
		{
			g_playerInteriorHandles[player->physicalPlayerIndex()] = 0;
			continue;
		}

		//TODO: replace other locations this is accessed.
		g_playerInteriorHandles[player->physicalPlayerIndex()] = netObj->At<int32_t>(0x218);
	}
}

// NetworkObjectMgr bitset update. Manages several int32 bitsets.
// Only some of these bitsets are relevant for OneSync along with the focus position call.
void ApplyNetworkObjectMgrPatches()
{
	auto location = hook::get_pattern("E8 ? ? ? ? 40 84 F6 74 ? 48 8B CF E8 ? ? ? ? 48 8B CF");
	hook::set_call(&g_origNetworkObjectMgr__UpdateBitsets, location);
	hook::call(location, CNetworkObjectMgr__UpdateBitsets);
}

// Update Player Focus Positions to support 128 players.
void CreatePlayerFocusHooks()
{
	MH_CreateHook(hook::get_pattern("0F A3 D0 0F 92 C0 88 06", -0x76), rage::GetPlayerFocusPosition, (void**)&g_origGetNetPlayerRelevancePosition);
	MH_CreateHook(hook::get_pattern("74 ? 4C 8D 44 24 ? C6 44 24 ? ? 48 8B D6", -68), rage::UpdatePlayerFocusPosition, (void**)&g_origUpdatePlayerFocusPosition);
}

void CreatePlayersNearPointHook()
{
	MH_CreateHook(hook::get_pattern("33 DB 0F 29 70 D8 49 8B F9 4D 8B F0", -0x1B), GetPlayersNearPoint, (void**)&g_origGetPlayersNearPoint);
}
