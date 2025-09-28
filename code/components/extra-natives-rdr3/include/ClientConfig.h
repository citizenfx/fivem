#pragma once

#include <bitset>
#include <cstdint>

enum class ClientConfigFlag : uint16_t
{
	WeaponsNoAutoReload = 0,
	UIVisibleWhenDead = 1,
	DisableDeathAudioScene = 2,
};

extern std::bitset<256> g_clientConfigBits;

void SetClientConfigFlag(ClientConfigFlag flag, bool enabled);
bool IsClientConfigEnabled(ClientConfigFlag flag);
