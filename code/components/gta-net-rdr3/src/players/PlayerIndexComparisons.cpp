#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <PatchUtils.h>
#include <PlayerLimits.h>

#include "PlayerPatches.h"

using rage::kMaxPlayers;

void ApplyNetObjectPlayerIndexPatches()
{
	// 32/31 32bit comparsions
	PatchValue<uint32_t>({
		// rage::netObject::DependencyThreadUpdate
		{ "41 BF ? ? ? ? 8A 8C 10", 2, 0x20, kMaxPlayers + 1}
	});

	PatchValue<uint8_t>({
		// rage::netObject::CanCreateWithNoGameObject
		{ "80 79 ? ? 73 ? 32 C0", 3, 0x20, kMaxPlayers + 1 },
		// rage::netObject::CanPassControl
		{ "3C ? 73 ? 3A 46", 1, 0x20, kMaxPlayers + 1},
		// rage::netObject::SetOwner
		{ "80 F9 ? 73 ? E8 ? ? ? ? 48 8B D8 EB", 2, 0x20, kMaxPlayers + 1},
		// rage::netObject::IsPendingOwnerChange
		{ "80 79 ? ? 0F 92 C0 C3 48 8B 91", 3, 0x20,  kMaxPlayers + 1 },

		// NetObjVehicle scene/viewport related. Array access is patched elsewhere.
		{ "49 8B 7E ? 48 85 FF 0F 84 ? ? ? ? 48 8B 2D", 23, 0x20, kMaxPlayers + 1 }
	});
}

// rage::netPlayerMgrBase
void ApplyNetPlayerMgrPatches()
{
	// Change count from 32 to 128
	PatchValue<uint32_t>({
		{ "C7 83 ? ? ? ? ? ? ? ? BA ? ? ? ? 48 89 AB", 6, 0x20, kMaxPlayers }
	});
}

void ApplyPlayerIndexComparisons()
{
	std::initializer_list<PatternClampPair> list = {
		//CNetGamePlayer::IsPhysical
		{ "80 79 ? ? 0F 92 C0 C3 48 89 5C 24", 3, false },
		//rage::netPlayer::IsPhysical
		{ "80 7B ? ? 73 ? B2", 3, false },

		// unk local player related
		{ "80 7D ? ? 48 8B F8 72 ? 32 C0", 3, false },

		// CPhysical::_CorrectSyncedPosition
		// TODO: needs extra patching
		//{ "40 80 FE ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84", 3, false },

		// getNetPlayerFromGamerHandleIfInSession
		{ "48 8B C8 48 8B D6 E8 ? ? ? ? 84 C0 75 ? FE C3", 19, false },

		// Related to CNetworkPopulationResetMgr
		//{ "40 80 FF ? 73 ? 48 8B 4E", 3, false},
		//{ "80 FB ? 72 ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 8B 7C 24", 2, false},
		//{ "80 FB ? 72 ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 83 C4 ? 41 5F 41 5E 41 5D 41 5C 5F C3 83 FA", 2, false },

		// Ped Combat related

		// CNetGamePlayer related, Has a potentially unsafe bitset
		//{ "80 FA ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 0F B6 C3 BA", 2, false },

		// CPedIntelligenceComponent::_unkTaskCombatRemoteShooting
		{ "80 FA ? 0F 83 ? ? ? ? 48 8B 05", 2, false },

		// CNetworkClearGangBountyEvent
		{ "80 3B ? 73 ? 48 8B CE", 2, false },

		// CNetObjProximityMigrateable::_getRelevancePlayers
		//{ "40 80 FF ? 0F 82 ? ? ? ? 0F 28 74 24 ? 4C 8D 5C 24 ? 49 8B 5B ? 48 8B C6", 3, false },

		//CNetObjGame::CanClone
		{ "80 7A ? ? 49 8B F8 48 8B DA 48 8B F1 72", 3, false},

		// getNetworkEntityOwner
		{ "80 F9 ? 72 ? 33 C0 C3 E9 ? ? ? ? 48 89 5C 24", 2, false },

		//FindNetworkPlayerPed
		{ "83 F9 ? 73 ? E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8", 2, false },

		//rage::netObjectIDMgr::TryToAllocateInitialObjectIDs, removes need for ScMultiplayerImpl size patches
		//Also removes the need to patch session entering logic.
		{ "80 79 ? ? 0F 83 ? ? ? ? 44 38 7B", 3, false },
		{ "80 FB ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 8A CB", 2, false },

		// rage::netObject::_doesPlayerHaveControlOverObject
		{ "80 79 ? ? 72 ? B0", 3, false },

		// Native Fixes
		{ "83 FB ? 73 ? 45 33 C0", 2, false }, // 0x862C5040F4888741
		{ "83 F9 ? 0F 83 ? ? ? ? B2", 2, false }, // 0x236321F1178A5446
		{ "83 F9 ? 73 ? 80 3D", 2, false }, // 0x93DC1BE4E1ABE9D1
		{ "48 85 C9 74 0F 83 FE 20", 7, false }, // 0x66B57B72E0836A76
		{ "83 F9 ? 73 ? B2", 2, false }, // 0x4CACA84440FA26F6
		{ "83 38 ? 48 8B 01 0F 92 C2", 2, false }, // 0x255A5EF65EDA9167

		// netObject vtable functions
		// Some bitsets are accessed here.
		//{ "49 81 ? C0 7A 02 00 E8 F3 ? ? 00 40 8A F0 ? 20", 16, false },

		//TODO: Investigate further if these patches are needed
		//{ "E8 76 68 E6 FD 84 C0 0F 84 96 00 00 00 ? ? ? 20", 16, false },
		//{ "48 8B CB E8 77 66 05 00 84 C0 74 41 40 80 FF 20", 15, false },

		// player iteration
		// used for gang/bounty logic. Not worth patching for now.
		//{ "80 FB ? 72 ? B0 ? 48 8B 5C 24 ? 48 83 C4", 2, false },

		// getPlayer
		{ "83 F9 ? 73 ? E8 ? ? ? ? 48 83 C4 ? C3 90 23 E0", 2, false },
		{ "80 F9 ? 73 ? E8 ? ? ? ? 48 8B D8 48 85 C0", 2, false },

		// Ped Group player comparsions
		{ "80 79 ? ? 73 ? 0F B6 41", 3, false },
		{ "83 F8 ? 76 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B8 ? ? ? ? 48 8D 0D ? ? ? ? 44 8D 4A ? E8 ? ? ? ? 84 C0 74 ? 48 69 C7", 2, true}
	};

	for (auto& entry : list)
	{
		auto location = hook::pattern(entry.pattern).count(1).get(0).get<uint8_t>(entry.offset);
		auto origVal = *location;
		assert(origVal == (entry.clamp ? 31 : 32));
		hook::put<uint8_t>(location, (entry.clamp ? kMaxPlayers : kMaxPlayers + 1));
	}
}

void ApplyPlayerIterationPatches()
{
	std::initializer_list<PatternClampPair> list = {
		// Player Cache Data Initalization
		{ "44 8D 41 ? 33 D2 4C 8D 0D", 3, true },
	};

	for (auto& entry : list)
	{
		auto location = hook::pattern(entry.pattern).count(1).get(0).get<uint8_t>(entry.offset);
		auto origVal = *location;
		assert(origVal == 32 || origVal == 31);
		hook::put<uint8_t>(location, origVal == 31 ? kMaxPlayers - 1 : kMaxPlayers);
	}
}

// CPedIntelligenceComponent, this has several atArrays that are sized for 32 players (or 1 if not-networked)
void ApplyPedIntelligencePatches()
{
	// Set maxPlayers networked to 127, as it increments the value by one later on
	hook::put<uint8_t>(hook::get_pattern("8D 43 ? 0F 45 D8 0F B7 87", 2), 0x7F);
	// Another atArray, set elsewhere inside of the constructor, Same behaviour as above.
	hook::put<uint8_t>(hook::get_pattern("83 E2 ? FF C2 E8 ? ? ? ? 66 3B 7B", 2), 0x7F);
}
