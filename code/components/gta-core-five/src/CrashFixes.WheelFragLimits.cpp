#include "StdInc.h"

#include "CrossBuildRuntime.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"
#include "Pool.h"

static int fragParenOffset = 0x400;
static int unkFlagOffset = 792;

static hook::cdecl_stub<void(void*, bool)> CObjectPopulation_DestroyObject([]
{
	return hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 45 33 F6 40 8A EA 48 8B D9 48 85 C9");
});

static void (*orig_CVehicle_Fix)(void*, bool, bool);

static void CVehicle_Fix(void* vehicle, bool resetFrag, bool allowNetwork)
{
	orig_CVehicle_Fix(vehicle, resetFrag, allowNetwork);

	const auto CObjectPool = rage::GetPool<void>("Object");

	for (int i = 0; i < CObjectPool->GetSize(); i++)
	{
		auto object = CObjectPool->GetAt(i);

		if (object == nullptr)
		{
			continue;
		}

		// Reference to the base entity this frag was created from (or nullptr)
		const void* fragParent = *(void**)((uintptr_t)object + fragParenOffset);

		// Is entity owned by frag cache
		const uint8_t isInFragCache = *(static_cast<uint8_t*>(object) + 0xCB) & 0x1F;

		// Is vehicle part or something
		const char unkFlags = *(char*)((uintptr_t)object + unkFlagOffset);

		if (fragParent == vehicle && isInFragCache == 2 && unkFlags < 0)
		{
			CObjectPopulation_DestroyObject(object, true);
		}
	}
}

static HookFunction hookFunction([]()
{
	orig_CVehicle_Fix = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8B 81"), CVehicle_Fix);

	if (xbr::IsGameBuildOrGreater<2802>())
	{
		fragParenOffset = 0x3E0;
		unkFlagOffset = 760;
	}
});
