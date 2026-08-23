#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <PatchUtils.h>
#include <PlayerLimits.h>

#include "PlayerPatches.h"

using rage::kMaxPlayers;

void ApplyPlayerDamageArrayPatches()
{
	constexpr size_t kDamageArraySize = sizeof(uint32_t) * (static_cast<size_t>(kMaxPlayers) + 1);
	uint32_t* damageArrayReplacement = (uint32_t*)hook::AllocateStubMemory(kDamageArraySize);
	memset(damageArrayReplacement, 0, kDamageArraySize);

	RelocateRelative((void*)damageArrayReplacement, {
		{ "48 8D 0D ? ? ? ? 44 21 35", 3 },
		{ "4C 8D 25 ? ? ? ? 41 83 3C B4", 3 },
		{ "48 8D 0D ? ? ? ? 85 DB 74", 3 },
		{ "48 8D 15 ? ? ? ? 8B 0C 82", 3 }
	});

	// Patch damage related comparisions
	PatchValue<uint8_t>({
		{"3C ? 73 ? 44 21 35", 1, 0x20, kMaxPlayers + 1},
		{"80 F9 ? 73 ? 0F B6 D1 48 8D 0D", 2, 0x20, kMaxPlayers + 1 },
		{"80 BB ? ? ? ? ? 73 ? 40 0F B6 C7", 6, 0x20, kMaxPlayers + 1 },
		//CWeaponDamageEvent::HandleReply
		{"40 80 FF ? 73 ? 40 38 3D", 3, 0x20, kMaxPlayers + 1},
		//CWeaponDamageEvent::_checkIfDead
		{ "80 3D ? ? ? ? ? 40 8A 68", 6, 0x20, kMaxPlayers + 1 },
	});
}

void ApplyPlayerDamageTrackerPatches()
{
	// 32/31 comparsions
	PatchValue<uint8_t>({
		// _addDamageDealtByPlayer
		{"80 7A ? ? 0F 28 F2 48 8B F2", 3, 0x20, kMaxPlayers + 1},
		// _getDamageDealtByPlayer
		{"80 7A ? ? 48 8B F9 72", 3, 0x20, kMaxPlayers + 1}
	});
}
