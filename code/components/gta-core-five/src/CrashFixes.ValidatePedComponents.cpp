#include "StdInc.h"

#include "console/Console.Base.h"
#include "console/Console.VariableHelpers.h"
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

static hook::cdecl_stub<bool(void* self, unsigned int slotId, int32_t drawblId, int32_t texId)> CPed_IsVariationInRange([]
{
	return hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 44 89 48 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 33 DB");
});

static bool allowEmptyHeadDrawableVal;

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
	// Validate that we are in range.
	if (drawblId != PV_NULL_DRAWBL && texId != 0 && !CPed_IsVariationInRange(self, slotId, drawblId, texId)) // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
	{
		console::PrintError("crash-mitigation", "CPed::SetVariation: Invalid variation (slotId: %d, drawblId: %u, texId: %u).\n", slotId, drawblId, texId);
		return false;
	}

	// Empty drawable cannot be set on the head component.
	if (!allowEmptyHeadDrawableVal && drawblId == PV_NULL_DRAWBL && slotId == PV_COMP_HEAD)
	{
		console::PrintError("crash-mitigation", "CPed::SetVariation: Attempted to set an empty drawable on the head component.\n");
		return false;
	}

	// Play it safe and don't allow a palette ID if we have a null drawable.
	if (drawblId == PV_NULL_DRAWBL && paletteId != 0)
	{
		console::PrintError("crash-mitigation", "CPed::SetVariation: Attempted to set a palette ID %u on a null drawable.\n", paletteId);
		paletteId = 0;
	}

	return orig_CPed_SetVariation(self, slotId, drawblId, drawblAltId, texId, paletteId, streamFlags, force);
}

static InitFunction initFunction([]
{
	static ConVar allowEmptyHeadDrawableVar("allowEmptyHeadDrawable", ConVar_UserPref, false, &allowEmptyHeadDrawableVal);
});

static HookFunction hookFunction([]
{
	orig_CPed_SetVariation = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 49 FF C6 49 83 FE ? 7C ? 4C 8D 9C 24")), CPed_SetVariation);
});
