#include <StdInc.h>
#include <Hooking.h>
#include "CrossBuildRuntime.h"

//
// This code patches an edge case when running `SET_PED_RANDOM_COMPONENT_VARIATION` native may crash the game.
// Crash happens because there's no checks that validates component variation unlike in other natives.
// In game build 2944.0 R* has added gen9 exclusive content into a shared dlc (i.e. not *-g9ec).
// We're adding validation check to the function that set ped component variation.
// 3407: do the same thing for random props
//

static hook::cdecl_stub<bool(void*, int, int)> _doesPedComponentDrawableExist([]()
{
	return xbr::IsGameBuildOrGreater<2699>() ? hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 8B 57 48")) : nullptr;
});

static hook::cdecl_stub<bool(void*, int, int, int)> _doesPedPropExist([]()
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F9 41 8B D8 8B F2 41 83 F8");
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

static void (*g_origSetPedProp)(void*, void*, int, int, int, int, void*, void*, int);
static void SetPedProp(void* unk, void* ped, int anchor, int propId, int textureId, int anchorOverride, void* unk2, void* unk3, int unk4)
{
	if (!_doesPedPropExist(ped, anchor, propId, textureId))
	{
		return;
	}

	g_origSetPedProp(unk, ped, anchor, propId, textureId, anchorOverride, unk2, unk3, unk4);
}


static HookFunction hookFunctionMetadataDep([]
{
	if (xbr::IsGameBuildOrGreater<2944>())
	{
		auto location =  hook::get_pattern("E8 ? ? ? ? EB 24 89 44 24 38");
		hook::set_call(&g_origSetStreamedPedComponentVariation, location);
		hook::call(location, SetStreamedPedComponentVariation);	
	}

	if (xbr::IsGameBuildOrGreater<3407>())
	{
		auto location =  hook::get_pattern("E8 ? ? ? ? 44 8A 4D ? 44 8A 45 ? 8A 55");
		hook::set_call(&g_origSetPedProp, location);
		hook::call(location, SetPedProp);
	}
});
