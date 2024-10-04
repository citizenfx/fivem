#include <StdInc.h>
#include <Hooking.h>
#include "CrossBuildRuntime.h"

//
// This code patches an edge case when running `SET_PED_RANDOM_COMPONENT_VARIATION` native may crash the game.
// Crash happens because there's no checks that validates component variation unlike in other natives.
// In game build 2944.0 R* has added gen9 exclusive content into a shared dlc (i.e. not *-g9ec).
// We're adding validation check to the function that set ped component variation.
//

static hook::cdecl_stub<bool(void*, int, int)> _doesPedComponentDrawableExist([]()
{
	return xbr::IsGameBuildOrGreater<2699>() ? hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 8B 57 48")) : nullptr;
});

static char (*g_origSetStreamedPedComponentVariation)(void*, void*, void*, int, int, int, int, int, int, char);

static char SetStreamedPedComponentVariation(void* ped, void* unk1, void* unk2, int compId, int drawableId, int unk3, int textureId, int paletteId, int unk4, char unk5)
{
	if (!_doesPedComponentDrawableExist(ped, compId, drawableId))
	{
		return false;
	}

	return g_origSetStreamedPedComponentVariation(ped, unk1, unk2, compId, drawableId, unk3, textureId, paletteId, unk4, unk5);
}

static HookFunction hookFunctionMetadataDep([]
{
	if (xbr::IsGameBuildOrGreater<2944>())
	{
		auto location =  hook::get_pattern("E8 ? ? ? ? EB 24 89 44 24 38");
		hook::set_call(&g_origSetStreamedPedComponentVariation, location);
		hook::call(location, SetStreamedPedComponentVariation);	
	}
});
