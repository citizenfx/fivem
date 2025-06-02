#include "StdInc.h"

#include "console/Console.Base.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

enum ePedVarComp
{
	PV_COMP_INVALID = -1,
	PV_COMP_HEAD = 0,
	PV_COMP_BERD,
	PV_COMP_HAIR,
	PV_COMP_UPPR,
	PV_COMP_LOWR,
	PV_COMP_HAND,
	PV_COMP_FEET,
	PV_COMP_TEEF,
	PV_COMP_ACCS,
	PV_COMP_TASK,
	PV_COMP_DECL,
	PV_COMP_JBIB,
	PV_MAX_COMP
};

static constexpr auto PV_NULL_DRAWBL = 0xFFFFFFFF;

static hook::cdecl_stub<bool(void* self, unsigned int slotId, unsigned int drawblId, unsigned int texId)> CPed_IsVariationValid([]
{
	return hook::get_pattern("48 8B 41 ? F6 80 ? ? ? ? ? 0F 85 ? ? ? ? E9");
});

static bool (*orig_CPed_SetVariation)(
void*,
ePedVarComp,
uint32_t,
uint32_t,
uint32_t,
uint32_t,
int32_t,
bool);

static bool CPed_SetVariation(
void* self,
ePedVarComp slotId,
uint32_t drawblId,
uint32_t drawblAltId,
uint32_t texId,
uint32_t paletteId,
int32_t streamFlags,
bool force)
{
	// Empty drawable cannot be set on head component.
	if (drawblId == PV_NULL_DRAWBL && slotId == PV_COMP_HEAD)
	{
		console::PrintError("crash-mitigation", "CPed::SetVariation: Attempted to set an empty drawable on head component.\n");
		return false;
	}

	// Verify if the variation is valid before setting it.
	if (!CPed_IsVariationValid(self, slotId, drawblId, texId))
	{
		console::PrintError("crash-mitigation", "CPed::SetVariation: Invalid variation (slotId: %d, drawblId: %u, texId: %u).\n", slotId, drawblId, texId);
		return false;
	}

	return orig_CPed_SetVariation(self, slotId, drawblId, drawblAltId, texId, paletteId, streamFlags, force);
}

static HookFunction hookFunction([]
{
	orig_CPed_SetVariation = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 49 FF C6 49 83 FE ? 7C ? 4C 8D 9C 24")), CPed_SetVariation);
});
