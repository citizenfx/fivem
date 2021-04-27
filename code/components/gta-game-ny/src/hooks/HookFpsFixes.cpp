#include <StdInc.h>
#include <Hooking.h>
#include <MinHook.h>

static int(__fastcall* g_origProcessBikeHandling)(void* self, void*, float a2, bool a3, int a4);

static int __fastcall Hook_ProcessBikeHandling(void* self, void* dummy, float a2, bool a3, int a4)
{
	constexpr const float minVal = (1.0f / 150.0f);

	if (a2 < minVal)
	{
		a2 = minVal;
	}

	return g_origProcessBikeHandling(self, dummy, a2, a3, a4);
}

static HookFunction hookFunc([]()
{
	MH_Initialize();
	MH_CreateHook(hook::pattern("7E 39 33 D2 3B CE 7D 0A 8B 87 80 0F 00 00 03 C2").count(2).get(1).get<void*>(-0x42), Hook_ProcessBikeHandling, (void**)&g_origProcessBikeHandling);
	MH_EnableHook(MH_ALL_HOOKS);
});
