#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

static void* (*g_origAllocateBroadPair)(void* pairPool, uint32_t objA, uint32_t objB);
static void* AllocateBroadPair(void* pairPool, uint32_t objA, uint32_t objB)
{
	auto result = g_origAllocateBroadPair(pairPool, objA, objB);
	
	if (result == nullptr)
	{
		trace("AllocateBroadPair pool filled\n");
	}
	
	return result;
}

static HookFunction hookFunction([]()
{
	g_origAllocateBroadPair = hook::trampoline(hook::get_pattern("48 83 EC ? 66 41 3B D0"), AllocateBroadPair);
});
