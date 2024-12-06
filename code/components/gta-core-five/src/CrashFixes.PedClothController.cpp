#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.FlexStruct.h>

#include "CrossBuildRuntime.h"

static void (*orig_CPedVariationStream__ApplyStreamPedFiles)(hook::FlexStruct* pPed, void* pNewVarData, void* pPSGfx);
static uint32_t off_CPed__m_CClothController;

// CPed holds non-owning pointers to each component's characterClothController.
// ApplyStreamPedFiles should update all of these, since they will stop existing soon afterwards.
// However, if a new component does not have a texture or drawable, the update loop will skip it without nulling the existing pointer.
static void CPedVariationStream__ApplyStreamPedFiles(hook::FlexStruct* pPed, void* pNewVarData, void* pPSGfx)
{
	void** controllers = pPed->At<void*[]>(off_CPed__m_CClothController);
	void* oldControllers[12];

	for (int i = 0; i < 12; ++i)
	{
		oldControllers[i] = controllers[i];
	}

	orig_CPedVariationStream__ApplyStreamPedFiles(pPed, pNewVarData, pPSGfx);

	for (int i = 0; i < 12; ++i)
	{
		if (controllers[i] && controllers[i] == oldControllers[i])
		{
			// We're still holding a pointer to the old controller!
			trace("Stale cloth controller pointer crash prevented [CFX-1735]\n");
			controllers[i] = nullptr;
		}
	}
}

static HookFunction hookFunction([]
{
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		return;
	}

	orig_CPedVariationStream__ApplyStreamPedFiles = hook::trampoline(hook::get_call(hook::get_pattern("4C 8B C3 48 8B D6 E8 ? ? ? ? 48 8B D7", 6)), &CPedVariationStream__ApplyStreamPedFiles);

	off_CPed__m_CClothController = *(uint32_t*)hook::get_pattern<char>("40 88 B8 ? ? ? ? 4A 89 BC E6", 11);
});
