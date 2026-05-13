#include "StdInc.h"

#include "Hooking.Patterns.h"

static HookFunction initFunction([]
{
	auto alphaStreetPrefetch = hook::get_pattern("48 63 CB 41 0F 18 44 8B");
	hook::nop(alphaStreetPrefetch, 25);

	auto alphaNormalPrefetch = hook::get_pattern("48 63 CF 41 0F 18 44 8B");
	hook::nop(alphaNormalPrefetch, 25);
});
