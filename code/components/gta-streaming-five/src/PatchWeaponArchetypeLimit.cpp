#include "StdInc.h"

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <Error.h>

struct atArrayHeader
{
	void* m_offset;
	uint16_t m_count;
	uint16_t m_size;
};

static void* g_weaponArchetypeStore;
static atArrayHeader* (*g_getWeaponArchetypeArray)(void* store, uint32_t storeKey);
static void* (*g_origAllocateWeaponArchetype)(void* definition, bool registerModel, uint32_t storeKey);

static void* AllocateWeaponArchetypeStub(void* definition, bool registerModel, uint32_t storeKey)
{
	if (auto array = g_getWeaponArchetypeArray(g_weaponArchetypeStore, storeKey))
	{
		// a zero m_size means this isn't an array we understand - stay out of the way
		if (array->m_size != 0 && array->m_count >= array->m_size)
		{
			AddCrashometry("weapon_archetype_limit", "%d", (int)array->m_size);

			FatalError(
				"Ran out of weapon archetype slots: all %d of them (MaxExtraWeaponModelInfos) are in use.\n"
				"\n"
				"Every add-on weapon registers one CWeaponModelInfo for each model it ships - the weapon itself "
				"plus every one of its component models - so this limit is hit long before the number of weapons "
				"looks suspicious, and the weaponarchetypes.meta being loaded when this happened (see the line "
				"right above this one in CitizenFX.log) is only the file that crossed the limit, not the file at "
				"fault.\n"
				"\n"
				"Load fewer add-on weapons, or raise MaxExtraWeaponModelInfos in gameconfig.xml.",
				(int)array->m_size);
		}
	}

	return g_origAllocateWeaponArchetype(definition, registerModel, storeKey);
}

static HookFunction hookFunction([]()
{
	auto allocPattern = hook::pattern("48 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 81 EC A0 00 00 00 48 8B E9 8A DA 48 8D 0D");

	if (allocPattern.size() != 1)
	{
		return;
	}

	auto location = allocPattern.get(0).get<char>(0);

	g_weaponArchetypeStore = hook::get_address<void*>(location + 0x20);
	g_getWeaponArchetypeArray = (decltype(g_getWeaponArchetypeArray))hook::get_call(location + 0x2A);

	g_origAllocateWeaponArchetype = hook::trampoline(location + 1, AllocateWeaponArchetypeStub);
});
