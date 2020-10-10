#include <StdInc.h>
#include <Hooking.h>
#include <EntitySystem.h>

#include <MinHook.h>

static void (*g_origCMapTypes__ConstructLocalExtensions)(void* mapTypes, uint32_t a2, fwArchetype* archetype, fwArchetypeDef* archetypeDef);

static void CMapTypes__ConstructLocalExtensions(void* mapTypes, uint32_t a2, fwArchetype* archetype, fwArchetypeDef* archetypeDef)
{
	// we don't want to load extensions for some models as they contain a breaking expression dict
	if (archetypeDef->assetName == 0x8b9108e7)
	{
		return;
	}

	return g_origCMapTypes__ConstructLocalExtensions(mapTypes, a2, archetype, archetypeDef);
}

static HookFunction hookFunction([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("33 DB 48 85 F6 7E 7A", -0x39), CMapTypes__ConstructLocalExtensions, (void**)&g_origCMapTypes__ConstructLocalExtensions);
	MH_EnableHook(MH_ALL_HOOKS);
});
