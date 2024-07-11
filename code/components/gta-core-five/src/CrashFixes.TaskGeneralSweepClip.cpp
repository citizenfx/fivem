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
	auto patterns = hook::pattern("48 8B ? 48 8B CB 48 89 44 24 20 E8 ? ? ? ? 84").count_hint(5);

	// They're all the same methods, so let's just use the first one.
	g_origCTaskGeneralSweep__Unk = (decltype(g_origCTaskGeneralSweep__Unk))hook::get_call(&patterns.get(2));

	/**
	 * Skip the first two matches.
	 * pattern 0
	 * pattern 1
	 * pattern 2 *
	 * pattern 3 *
	 * pattern 4 *
	 */

	for (auto i = 2; i <= 4; i++)
	{
		hook::call(patterns.get(i).get<void>(0xB), CTaskGeneralSweep__Unk);
	}
});
