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

static hook::cdecl_stub<void(rage::fwExtensionList*, rage::fwExtension*)> addExtension([]()
{
	return hook::get_pattern("48 89 5C 24 08 57 48 83 EC 20 48 8B F9 48 8B DA B9 10 00 00 00");
});

static hook::cdecl_stub<void*(rage::fwExtensionList*, uint32_t)> getExtension([]()
{
	return hook::get_pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 8B FA 83 FA 3E");
});

void rage::fwExtensionList::Add(rage::fwExtension* extension)
{
	return addExtension(this, extension);
}

void* rage::fwExtensionList::Get(uint32_t id)
{
	return getExtension(this, id);
}

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, rage::fwModelId& id)> getArchetype([]()
{
	return hook::get_call(hook::pattern("8B 4E 08 C1 EB 05 80 E3 01 E8").count(1).get(0).get<void>(9));
});

fwArchetype* rage::fwArchetypeManager::GetArchetypeFromHashKey(uint32_t hash, fwModelId& id)
{
	return getArchetype(hash, id);
}
