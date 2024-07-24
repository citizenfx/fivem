#include <StdInc.h>

#include <Hooking.h>

static bool (*g_origCTaskGeneralSweep__Unk)(void* task, void* clip, float* unk1, float* unk2, float* unk3);
static bool CTaskGeneralSweep__Unk(void* task, void* clip, float* unk1, float* unk2, float* unk3)
{
	if (!clip)
		return false;

	return g_origCTaskGeneralSweep__Unk(task, clip, unk1, unk2, unk3);
}

static HookFunction hookFunction([]
{
	auto firstCallLoc = hook::get_pattern("48 8B D5 48 8B CB 48 89 44 24 ? E8 ? ? ? ? 84 C0 75", 11);
	auto secondCallLoc = hook::get_pattern("48 8B D5 48 8B CB 48 89 44 24 ? E8 ? ? ? ? 84 C0 74", 11);
	auto thirdCallLoc = hook::get_pattern("48 8B D7 48 8B CB 48 89 44 24 ? E8 ? ? ? ? 84 C0", 11);

	// They're all the same methods, so let's just use the first one.
	g_origCTaskGeneralSweep__Unk = (decltype(g_origCTaskGeneralSweep__Unk))hook::get_call(firstCallLoc);

	hook::call(firstCallLoc, CTaskGeneralSweep__Unk);
	hook::call(secondCallLoc, CTaskGeneralSweep__Unk);
	hook::call(thirdCallLoc, CTaskGeneralSweep__Unk);
});
