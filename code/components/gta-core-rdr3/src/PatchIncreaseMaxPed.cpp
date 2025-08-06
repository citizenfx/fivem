#include <StdInc.h>

#include <Pool.h>
#include <PoolSizesState.h>
#include "Hooking.h"
#include "Hooking.Stubs.h"

//
// There is a max limit of 110 ped and 110 ped components. These ped pools are shared with every type of ped (Horses, NPC's, Animals) with the exception of player
// The game hardcode this logic and has some pool size checks on startup, fatally erroring if a certain size is exceeded. Limiting the ability to have more then 110 peds
// This patch resolves this by improving logic to account for the increase size of "CNetObjPedBase" pool and removing a ped pool size check that would otherwise fatally error.
//

static HookFunction hookFunction([]()
{
	constexpr size_t kDefaultMaxPeds = 110;

	int64_t increaseSize = 0;

	// We use "CNetObjPedBase" as the increase request for peds and all other components.
	auto sizeIncreaseEntry = fx::PoolSizeManager::GetIncreaseRequest().find("CNetObjPedBase");
	if (sizeIncreaseEntry != fx::PoolSizeManager::GetIncreaseRequest().end())
	{
		increaseSize = sizeIncreaseEntry->second;
	}

	// Don't fatally error if "Peds" pool is greater then 160.
	hook::put<uint8_t>(hook::get_pattern("76 ? BA ? ? ? ? 41 B8 ? ? ? ? 83 C9 ? E8 ? ? ? ? 33 D2 8B CF"), 0xEB);
	
	// Set total desired peds.
	*hook::get_address<uint32_t*>(hook::get_pattern("89 05 ? ? ? ? C6 05 ? ? ? ? ? 89 15", 2)) = kDefaultMaxPeds + increaseSize;
	
	// Set max amount of peds.
	*hook::get_address<uint32_t*>(hook::get_pattern("89 05 ? ? ? ? 89 05 ? ? ? ? 89 05 ? ? ? ? C6 05", 2)) = kDefaultMaxPeds + increaseSize;

	// Allow registration of script/mission peds up to and past the 110 limit.
	hook::put<uint32_t>(hook::get_pattern("BB ? ? ? ? E9 ? ? ? ? E8 ? ? ? ? 48 8B 0D", 1), kDefaultMaxPeds + increaseSize);
});
