#include "StdInc.h"

#include "CrossBuildRuntime.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"
#include "Hooking.FlexStruct.h"
#include "Pool.h"

static uint32_t morphControllerFieldOffset = 0x0;
static void (*orig_SwitchLOD)(hook::FlexStruct* self, int, int, int, bool);

static void SwitchLOD_Fix(hook::FlexStruct* self, int a1, int a2, int a3, bool a4)
{
	const auto morphController = self->At<void*>(morphControllerFieldOffset);
	if (!morphController)
	{
		return;
	}

	orig_SwitchLOD(self, a1, a2, a3, a4);
}

static HookFunction hookFunction([]()
{
	morphControllerFieldOffset = *(uint8_t*)hook::get_pattern<char>("48 8B 71 ? 49 63 C0", 3);
	orig_SwitchLOD = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 71 ? 49 63 C0"), SwitchLOD_Fix);
});
