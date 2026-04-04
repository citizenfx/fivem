#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

static uint32_t* (*g_CWeaponInfo_GetClipsetForWeaponSwap)(hook::FlexStruct*, uint32_t*, hook::FlexStruct*, uint32_t);

static uint32_t* CWeaponInfo_GetClipsetForWeaponSwap(hook::FlexStruct* thisPtr, uint32_t* outClipSet, hook::FlexStruct* ped, uint32_t unkEnum)
{
	// NOTE: thisPtr (rcx) is nullptr when CClonedTaskWeaponInfo->m_Weapon is not a valid weapon
	if (!thisPtr)
	{
		*outClipSet = 0;
		return outClipSet;	
	}

	return g_CWeaponInfo_GetClipsetForWeaponSwap(thisPtr, outClipSet, ped, unkEnum);
}

static HookFunction hookFunction([]()
{
	// A null pointer dereference occurs during processing of a synchronized weapon swap task.
	// If a serialized weapon is invalid, this ends up with the weapon info being nullptr.
	// 
	// When the nullptr is passed to CWeaponInfo_GetClipsetForWeaponSwap,
	// the game attempts to access thisPtr->m_Name (0x10) without validation, causing a crash.
	//
	// This hook prevents the crash by checking if thisPtr is nullptr and returning a safe fallback value.

	g_CWeaponInfo_GetClipsetForWeaponSwap = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B E9 49 8B F8 48 8B DA 48 8B F1 4D 85 C0 0F 84 ? ? ? ? 41 F7 80"), CWeaponInfo_GetClipsetForWeaponSwap);
});
