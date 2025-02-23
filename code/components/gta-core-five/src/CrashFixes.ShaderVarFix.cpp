#include "StdInc.h"

#include "CrossBuildRuntime.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"
#include "Pool.h"

static void (*orig_shaderGroup_SetVar)(hook::FlexStruct* self, uint64_t a2, uint64_t a3);

static void shaderGroup_SetVar(hook::FlexStruct* self, uint64_t a2, uint64_t a3)
{
	void* valuePairs = self->At<void*>(0x20);
	if (!valuePairs)
	{
		return;
	}

	orig_shaderGroup_SetVar(self, a2, a3);
}

static HookFunction hookFunction([]()
{
	orig_shaderGroup_SetVar = hook::trampoline(hook::get_pattern("85 D2 0F 84 ? ? ? ? 48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 8D 42"), shaderGroup_SetVar);
});
