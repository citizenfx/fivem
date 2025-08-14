#include <StdInc.h>

#include <Pool.h>
#include <PoolSizesState.h>
#include "Hooking.h"
#include "Hooking.Stubs.h"

static HookFunction hookFunction([]()
{
	constexpr size_t kDefaultMaxObjects = 60;

	int64_t increaseSize = 0;

	// We use "CNetObjObject" as the increase request for objects and all other components.
	auto sizeIncreaseEntry = fx::PoolSizeManager::GetIncreaseRequest().find("CNetObjObject");
	if (sizeIncreaseEntry != fx::PoolSizeManager::GetIncreaseRequest().end())
	{
		increaseSize = sizeIncreaseEntry->second;
	}

	// Set total desired objects.
	hook::put<uint32_t>(hook::get_pattern("C7 05 ? ? ? ? ? ? ? ? 89 05 ? ? ? ? 89 0D", 0x6), kDefaultMaxObjects + increaseSize);

	// Set max amount of objects
	*hook::get_address<uint32_t*>(hook::get_pattern("89 0D ? ? ? ? 8D 41 ? 44 89 05", 2)) = kDefaultMaxObjects + increaseSize;
	*hook::get_address<uint32_t*>(hook::get_pattern("89 0D ? ? ? ? 89 0D ? ? ? ? 8D 41 ? 44 89 05", 2)) = kDefaultMaxObjects + increaseSize;
	*hook::get_address<uint32_t*>(hook::get_pattern("89 0D ? ? ? ? 89 0D ? ? ? ? 89 0D ? ? ? ? 8D 41", 2)) = kDefaultMaxObjects + increaseSize;
	
	// Allow registration of script/mission objects up to and past the 60 limit.
	hook::put<uint32_t>(hook::get_pattern("BB ? ? ? ? EB ? BB ? ? ? ? EB ? BB ? ? ? ? EB ? 8B CF", 1), kDefaultMaxObjects + increaseSize);
});