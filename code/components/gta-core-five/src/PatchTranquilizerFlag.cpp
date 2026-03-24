#include "StdInc.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"
#include "XBRVirtual.h"

static ptrdiff_t pedIntelligenceOffset;
static ptrdiff_t eventScannerOffset;
static ptrdiff_t tranquilizeDamageTimeOffset;
static ptrdiff_t pedFlagsOffset;
static ptrdiff_t tranquilizedFlagOffset;

static void (*g_origResurrectPed)(hook::FlexStruct* self, void* coords, float heading, bool a4, bool a5);

static void ResurrectPed(hook::FlexStruct* ped, void* coords, float heading, bool a4, bool a5)
{
	g_origResurrectPed(ped, coords, heading, a4, a5);

	// Reset tranquilizer damage time:
	// If not cleared, the ped will immediately die again after resurrection,
	// because the game still thinks the tranquilizer effect is active.
	hook::FlexStruct* pedIntelligence = ped->At<hook::FlexStruct*>(pedIntelligenceOffset);
	pedIntelligence->Set<uint32_t>(eventScannerOffset + tranquilizeDamageTimeOffset, 0);

	// Clear tranquilized flag:
	// Prevents the ped from being stuck in an infinite loop of
	// "resurrect → still tranquilized → die again".
	uint32_t flags = ped->Get<uint32_t>(pedFlagsOffset);
	flags &= ~tranquilizedFlagOffset;
	ped->Set<uint32_t>(pedFlagsOffset, flags);
}

static HookFunction hookFunction([]()
{
	// The tranquilizer weapon (WEAPON_TRANQUILIZER) was only introduced in the Diamond Casino DLC (build 2060+).
	// Also, this patch could be skipped for newer builds higher than 3570 because R* already patched the issue.
	if(xbr::IsGameBuild<1604>() || xbr::GetGameBuild() > 3570)
	{
		return;
	}
	
	pedIntelligenceOffset       = *reinterpret_cast<int32_t*>(hook::get_pattern("48 8B 83 ? ? ? ? 40 8A D6 F3 0F 10 15", 0x3));
	eventScannerOffset          = *reinterpret_cast<int32_t*>(hook::get_pattern("48 81 C1 ? ? ? ? 48 8B D3 40 8A F0", 0x3));
	tranquilizeDamageTimeOffset = *reinterpret_cast<int32_t*>(hook::get_pattern("41 89 B6 ? ? ? ? E8 ? ? ? ? 48 8D 15", 0x3));
	pedFlagsOffset              = *reinterpret_cast<int32_t*>(hook::get_pattern("8B 83 ? ? ? ? F7 D0 25 ? ? ? ? 31 83 ? ? ? ? 8B 83 ? ? ? ? F7 D0 25 ? ? ? ? 31 83 ? ? ? ? 44 89 67", 0x2));
	tranquilizedFlagOffset      = *reinterpret_cast<int32_t*>(hook::get_pattern("25 ? ? ? ? 31 83 ? ? ? ? 8B 83 ? ? ? ? F7 D0 25 ? ? ? ? 31 83 ? ? ? ? 44 89 67", 0x1));

	g_origResurrectPed = hook::trampoline(
		hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B AF ? ? ? ? 48 85 ED 0F 84")),
		ResurrectPed
	);
});
