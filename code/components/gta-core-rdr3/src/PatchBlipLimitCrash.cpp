#include <StdInc.h>
#include <Hooking.h>
#include <MinHook.h>
#include <Pool.h>

static void* (*g_origCreateWaypoint)(void* waypointController, uint32_t* blipHash);
static void* CreateWaypoint(void* waypointController, uint32_t* blipHash)
{
	static auto pool = rage::GetPoolBase("fwuiBlip");

	if (pool->GetCountDirect() >= pool->GetSize())
	{
		trace("Prevented crash: Blip pool 'fwuiBlip' is full (%d out of %d slots used)\n", pool->GetCountDirect(), pool->GetSize());
		return 0;
	}

	return g_origCreateWaypoint(waypointController, blipHash);
}

static HookFunction hookFunction([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("45 33 C0 44 38 05 ? ? ? ? 41 0F 95 C0"), CreateWaypoint, (void**)&g_origCreateWaypoint);
	MH_EnableHook(MH_ALL_HOOKS);
});
