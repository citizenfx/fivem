#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <MinHook.h>
#include <NetLibrary.h>
#include <netObject.h>
#include <ICoreGameInit.h>
#include <CoreConsole.h>
#include <netPlayerManager.h>
#include <netBitsets.h>
#include <netObjectMgr.h>

// RDR3 makes extensive use of 32 sized ints that are used as bitsets with flags being the players physicalIndexes.
// This causes unintended behaviour with >32 indexes as they will try reading/writing past the bounds of the bitset.
// Usually leading to crashes or other problems. As a result, we create an extra set of int32's for indexes >32 to make use of.
// Restoring original functionality

static const uint8_t kOriginalPlayers = 32;
static const uint8_t kMaxPlayers = 128;
static const uint16_t kMaxObjects = std::numeric_limits<uint16_t>::max();

extern ICoreGameInit* icgi;

// Extend bitsets to handle more then 32 players at a time
constexpr size_t kBitsSize = 32;
constexpr size_t kBitsetArraySize = kMaxPlayers / kBitsSize;
using PlayerBitset = uint32_t[kMaxPlayers][kBitsetArraySize];
using ObjectBitset = uint32_t[kMaxObjects][kBitsetArraySize];

// Bitset related to remote players in the clients scope.
uint32_t g_remotePlayersInScope;

// Bitsets for Objects
ObjectBitset g_netObjAcknowledgement;
ObjectBitset g_netObjScopeState;
ObjectBitset g_netObjPlayerTargetting;
// Bitsets for Players

rage::atPlayerBitsets g_playerFocusPositionUpdateBitset;
rage::atPlayerBitsets g_physicalPlayers;

void ClearNetObject(rage::netObject* object)
{
	uint16_t objectId = object->GetObjectId();
	memset(g_netObjAcknowledgement[objectId], 0, sizeof(g_netObjAcknowledgement[objectId]));
	memset(g_netObjPlayerTargetting[objectId], 0, sizeof(g_netObjPlayerTargetting[objectId]));
}

// Initialize bitsets to match base game behaviour
void SetupNetObject(rage::netObject* object)
{
	uint16_t objectId = object->GetObjectId();

	// The game sets every bit in the bitflag to 1 for ped targetting.
	for (int i = 0; i < kBitsetArraySize; i++)
	{
		g_netObjPlayerTargetting[objectId][i] = 0xFFFFFFFF;
	}
}

// netObject Player Acknowledgments. In onesync these are only set in rare cases (outside of player 31)
static bool (*g_origNetobjIsPlayerAcknowledged)(rage::netObject*, CNetGamePlayer*);
static bool netObject__IsPlayerAcknowledged(rage::netObject* object, CNetGamePlayer* player)
{
	uint8_t physicalIndex = player->physicalPlayerIndex();
	if (!icgi->OneSyncEnabled || physicalIndex < 32)
	{
		bool result = g_origNetobjIsPlayerAcknowledged(object, player);
		return result;
	}

	int index = (physicalIndex / 32) - 1;
	int bit = physicalIndex % 32;

	if (physicalIndex > kMaxPlayers)
	{
		return false;
	}

	uint32_t bitset = g_netObjAcknowledgement[object->GetObjectId()][index];

	return (bitset >> bit) & 1;
}

static uint32_t (*g_origNetObjectSetPlayerAcknowledged)(rage::netObject*, CNetGamePlayer*, bool);
static uint32_t netObject__setPlayerAcknowledged(rage::netObject* object, CNetGamePlayer* player, bool state)
{
	uint8_t physicalIndex = player->physicalPlayerIndex();

	// Allow non-onesync and index 31 to behave as intended
	if (!icgi->OneSyncEnabled || physicalIndex < 0x20)
	{
		return g_origNetObjectSetPlayerAcknowledged(object, player, state);
	}

	int index = (physicalIndex / kBitsSize) - 1;
	int bit = physicalIndex % kBitsSize;

	if (physicalIndex > kMaxPlayers)
	{
		return false;
	}

	if (state)
	{
		g_netObjAcknowledgement[object->GetObjectId()][index] |= (1 << bit);
	}
	else
	{
		g_netObjAcknowledgement[object->GetObjectId()][index] &= ~(1 << bit);
	}

	return g_netObjAcknowledgement[object->GetObjectId()][index];
}

// netObject Pending removal, we can stub this in onesync
static uint32_t (*g_origSetPendingRemovalByPlayer)(rage::netObject*, CNetGamePlayer*, uint8_t);
static uint32_t netObject__setPendingRemovalByPlayer(rage::netObject* object, CNetGamePlayer* player, uint8_t newState)
{
	uint8_t physicalIndex = player->physicalPlayerIndex();
	if (!icgi->OneSyncEnabled || physicalIndex < 0x20)
	{
		return g_origSetPendingRemovalByPlayer(object, player, newState);
	}

	return 0;
}

static bool (*g_origIsPendingRemovalByPlayer)(rage::netObject*, CNetGamePlayer*);
static bool netObject__isPendingRemovalByPlayer(rage::netObject* object, CNetGamePlayer* player)
{
	uint8_t physicalIndex = player->physicalPlayerIndex();
	// Allow non-onesync and index 31 to behave as intended
	if (!icgi->OneSyncEnabled || physicalIndex < 0x20)
	{
		return g_origIsPendingRemovalByPlayer(object, player);
	}

	// Onesync doesn't care about the result
	return false;
}

// netObject player targetting
static bool (*g_origSetCanBeTargetted)(rage::netObject*, uint8_t, bool);
static bool netObject__setCanBeTargetted(rage::netObject* object, uint8_t physicalIndex, bool state)
{
	if (!icgi->OneSyncEnabled || physicalIndex < 0x20)
	{
		return g_origSetCanBeTargetted(object, physicalIndex, state);
	}

	int index = (physicalIndex / kBitsSize) - 1;
	int bit = physicalIndex % kBitsSize;

	if (physicalIndex > kMaxPlayers)
	{
		return false;
	}

	if (state)
	{
		g_netObjPlayerTargetting[object->GetObjectId()][index] |= (1 << bit);
	}
	else
	{
		g_netObjPlayerTargetting[object->GetObjectId()][index] &= ~(1 << bit);
	}
}

static bool (*g_origCanBeTargetted)(rage::netObject*, uint8_t);
static bool netObject__canBeTargetted(rage::netObject* object, uint8_t physicalIndex)
{
	if (!icgi->OneSyncEnabled || physicalIndex < 0x20)
	{
		return g_origCanBeTargetted(object, physicalIndex);
	}

	if (physicalIndex > kMaxPlayers)
	{
		return false;
	}

	int index = (physicalIndex / 32) - 1;
	int bit = physicalIndex % 32;
	uint32_t bitset = g_netObjPlayerTargetting[object->GetObjectId()][index];
	return (bitset >> bit) & 1;
}

static uint32_t (*g_netObject__setScopeState)(rage::netObject*, CNetGamePlayer*);
static uint32_t netObject__setScopeState(rage::netObject* object, CNetGamePlayer* player)
{
	uint8_t physicalIndex = player->physicalPlayerIndex();
	if (!icgi->OneSyncEnabled || physicalIndex < 0x20)
	{
		return g_netObject__setScopeState(object, player);
	}

	return 0;
}

static HookFunction hookFunction([]()
{
	// Avoid writing to netPlayerMgrBase remotePlayer bitset with extended physical indexes.
	{
		auto location = hook::get_pattern<char>("48 8B CF FF 50 ? 48 8B C8 E8 ? ? ? ? 84 C0 74 ? 0F B6 4F", 0x18);

		static struct : jitasm::Frontend
		{
			uintptr_t successAddr;
			uintptr_t failAddr;

			void Init(uintptr_t successAddress, uintptr_t failAddress)
			{
				successAddr = successAddress;
				failAddr = failAddress;
			}

			virtual void InternalMain() override
			{
				// Original Code
				movzx(edx, byte_ptr[rdi + 0x19]);
				mov(edx, ecx);

				// Only use original behaviour if the index is 32 safe.
				cmp(edx, kOriginalPlayers);
				jge("Fail");

				L("Fail");
				mov(rax, failAddr);
				jmp(rax);

				//TODO: Populate our own bitset, and feed it to natives that require this functionality
				mov(rax, successAddr);
				jmp(rax);
			}
		} patchStub;

		const uintptr_t retnSuccess = (uintptr_t)location + 6;
		const uintptr_t retnFail = (uintptr_t)location + 30;

		hook::nop(location, 6);
		patchStub.Init(retnSuccess, retnFail);
		hook::jump(location, patchStub.GetCode());
	}

	MH_Initialize();
	// Creation Acknowledgement
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 40 8A FB 84 C0 74")), netObject__IsPlayerAcknowledged, (void**)&g_origNetobjIsPlayerAcknowledged);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? EB ? 47 85 0C 86")), netObject__setPlayerAcknowledged, (void**)&g_origNetObjectSetPlayerAcknowledged);
	// Removal
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 33 FF 84 C0 75 ? 8A 4B")), netObject__isPendingRemovalByPlayer, (void**)&g_origIsPendingRemovalByPlayer);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? EB ? 45 84 FF 74 ? 48 8D 4C 24")), netObject__setPendingRemovalByPlayer, (void**)&g_origSetPendingRemovalByPlayer);
	// Scope
	MH_CreateHook(hook::get_pattern("8B FB 49 8B CE 48 C1 EF", -93), netObject__setScopeState, (void**)&g_netObject__setScopeState);

	// Player targetting
	{
		auto location = hook::get_pattern("E8 ? ? ? ? 45 33 C9 84 C0 41 0F 94 C5");
		hook::set_call(&g_origCanBeTargetted, location);

		// Writing over the original call instruction has slightly less overhead then Minhook.
		hook::call(location, netObject__canBeTargetted);
		hook::call(hook::get_pattern("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 41 F6 46"), netObject__canBeTargetted);
		hook::call(hook::get_pattern("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 8A 5C 24"), netObject__canBeTargetted);
	}

	MH_CreateHook(hook::get_pattern("84 C0 74 ? 4C 8B C7 40 0F B6 C5", -0x47), netObject__setCanBeTargetted, (void**)&g_origSetCanBeTargetted);

	MH_EnableHook(MH_ALL_HOOKS);
});
