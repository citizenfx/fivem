#include <StdInc.h>
#include <Hooking.h>

#include <EntitySystem.h>

fwArchetype* GetArchetypeSafe(uint32_t archetypeHash, uint64_t* archetypeUnk);

static hook::cdecl_stub<fwArchetype*(uint32_t* archetypeUnk)> getArchetypeFromUnk([]()
{
	return hook::get_call(hook::get_pattern("89 44 24 30 E8 ? ? ? ? 48 8B CB 48 8B F0 E8", 4));
});

static hook::cdecl_stub<bool(uint32_t* mi)> hasModelLoaded([]()
{
	return hook::get_call(hook::get_pattern("25 FF FF FF 3F 89 45 6F E8 ? ? ? ? 84 C0", 8));
});

DLL_EXPORT fwEvent<PopulationCreationState*> OnCreatePopulationPed;

static void*(*g_createPopulationPed)(int32_t mi, float* position, float, void*, char a5, unsigned int a6, __int64 a7, char a8, char a9, char a10, char a11, __int64 a12, char a13, char a14, unsigned int a15, int a16, __int64 a17, __int64 a18, char a19, int a20);

static void* CreatePopulationPedWrap(uint32_t mi, float* position, float a3, void* a4, char a5, unsigned int a6, __int64 a7, char a8, char a9, char a10, char a11, __int64 a12, char a13, char a14, unsigned int a15, int a16, __int64 a17, __int64 a18, char a19, int a20)
{
	auto archetype = getArchetypeFromUnk(&mi);
	uint32_t modelHash = 0;

	if (archetype)
	{
		modelHash = archetype->hash;
	}

	// create event state
	PopulationCreationState creationState;
	creationState.allowed = true;
	creationState.model = modelHash;
	creationState.position[0] = position[0];
	creationState.position[1] = position[1];
	creationState.position[2] = position[2];

	// trigger a creation event
	OnCreatePopulationPed(&creationState);

	// apply changed data
	if (creationState.model != modelHash)
	{
		uint64_t archetypeUnk = mi;
		GetArchetypeSafe(creationState.model, &archetypeUnk);

		uint32_t at = archetypeUnk;
		if (!hasModelLoaded(&at))
		{
			return nullptr;
		}

		mi = archetypeUnk & 0xFFFF;
	}

	position[0] = creationState.position[0];
	position[1] = creationState.position[1];
	position[2] = creationState.position[2];

	// if allowed, create the ped
	if (creationState.allowed)
	{
		return g_createPopulationPed(mi, position, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
	}

	return nullptr;
}

#include <MinHook.h>

static HookFunction hookFunction([]()
{
	auto location = hook::get_call(hook::get_pattern("44 89 7C 24 28 88 44 24 20 44 88 2D ? ? ? ? E8", 16));
	MH_Initialize();
	MH_CreateHook(location, CreatePopulationPedWrap, (void**)&g_createPopulationPed);
	MH_EnableHook(MH_ALL_HOOKS);

#if 0
	OnCreatePopulationPed.Connect([](PopulationCreationState* state)
	{
		trace("Creating population ped at (%g, %g, %g): %08x\n", state->position[0], state->position[1], state->position[2], state->model);

		state->model = 0x4E8F95A2;
	});
#endif
});
