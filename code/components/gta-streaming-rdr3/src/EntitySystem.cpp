#include <StdInc.h>
#include <EntitySystem.h>

#include <Hooking.h>

static hook::cdecl_stub<fwEntity* (int handle)> getScriptEntity([]()
{
	return hook::pattern("45 8B C1 41 C1 F8 08 45 38 0C 00 75 ? 8B 42 ? 41 0F AF C0").count(1).get(0).get<void>(-81);
});

fwEntity* rage::fwScriptGuid::GetBaseFromGuid(int handle)
{
	return getScriptEntity(handle);
}

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, rage::fwModelId& archetypeUnk)> getArchetype([]()
{
	return hook::get_call(hook::pattern("8B 4E 08 C1 EB 05 80 E3 01 E8").count(1).get(0).get<void>(9));
});

fwArchetype* rage::fwArchetypeManager::GetArchetypeFromHashKey(uint32_t hash, fwModelId& id)
{
	return getArchetype(hash, id);
}
