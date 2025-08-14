#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

static hook::FlexStruct* (*g_orig_River__GetRiverEntity)(int RiverEntity);

static void (*g_orig_GetBoundingBoxZForRiverEntity)(int RiverEntityIndex, float* MinZ, float* MaxZ);
static void GetBoundingBoxZForRiverEntity(int RiverEntityIndex, float* MinZ, float* MaxZ)
{
	hook::FlexStruct* entity = g_orig_River__GetRiverEntity(RiverEntityIndex);
	if (!entity)
		return;
	if(!entity->At<void*>(0x20)) // Entity archetype
		return;

	g_orig_GetBoundingBoxZForRiverEntity(RiverEntityIndex, MinZ, MaxZ);
}

static HookFunction hookFunction([]
{
	static auto RiverEntity = hook::get_pattern("E8 ? ? ? ? 48 8B 48 ? 4C 8D 44 24"); // call for GetRiverEntity
	g_orig_River__GetRiverEntity = (decltype(g_orig_River__GetRiverEntity))hook::get_call(RiverEntity);

	g_orig_GetBoundingBoxZForRiverEntity = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 57 48 81 EC A0 00 00 00 49 8B F8"), GetBoundingBoxZForRiverEntity);
});
