/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <Hooking.h>
#include <MissionCleanup.h>
#include <MinHook.h>

CMissionCleanup* GetMissionCleanupInstance(CMissionCleanup* instance)
{
	// make it a stack argument in case it'd be undefined behavior otherwise
	CMissionCleanup* instancePtr = instance;

	//HookCallbacks::RunCallback(StringHash("mCleanup"), &instancePtr);
	CMissionCleanup::OnQueryMissionCleanup(instancePtr);

	return instancePtr;
}

int(__fastcall* g_origAddEntityToList)(CMissionCleanup* a1, void* dummy, int a2, char a3, int a4, int a5, int a6);

static int __fastcall AddEntityToListHook(CMissionCleanup* a1, void* dummy, int a2, char a3, int a4, int a5, int a6)
{
	return g_origAddEntityToList(GetMissionCleanupInstance(a1), dummy, a2, a3, a4, a5, a6);
}

int(__fastcall* g_origRemoveEntityFromList)(CMissionCleanup* a1, void* dummy, int a2, char a3, int a4, int a5);

static int __fastcall RemoveEntityFromListHook(CMissionCleanup* a1, void* dummy, int a2, char a3, int a4, int a5)
{
	return g_origRemoveEntityFromList(GetMissionCleanupInstance(a1), dummy, a2, a3, a4, a5);
}

int(__fastcall* g_origCheckIfCollisionHasLoadedForMissionObjects)(CMissionCleanup* a1);

static int __fastcall CheckIfCollisionHasLoadedForMissionObjectsHook(CMissionCleanup* a1)
{
	return g_origCheckIfCollisionHasLoadedForMissionObjects(GetMissionCleanupInstance(a1));
}

static void RunMissionCleanupCollisionChecks()
{
	CMissionCleanup::OnCheckCollision();
}

fwEvent<CMissionCleanup*&> CMissionCleanup::OnQueryMissionCleanup;
fwEvent<> CMissionCleanup::OnCheckCollision;

static HookFunction hookFunction([] ()
{

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("33 D2 83 C0 04 33 F6 33 F", -0x0F), AddEntityToListHook, (void**)&g_origAddEntityToList); // mission cleanup 'add script entry'
	MH_CreateHook(hook::get_pattern("53 8A 5C 24 0C 55 0F B6 C3"), RemoveEntityFromListHook, (void**)&g_origRemoveEntityFromList); // mission cleanup 'remove script entry'
	MH_CreateHook(hook::get_pattern("BB 00 01 00 00 90", -0x0A), CheckIfCollisionHasLoadedForMissionObjectsHook, (void**)&g_origCheckIfCollisionHasLoadedForMissionObjects); // mission cleanup 'check collision'
	MH_EnableHook(MH_ALL_HOOKS);

	// per-tick mission cleanup collision check
	static hook::inject_call<void, int> scriptTickCollision((ptrdiff_t)hook::get_pattern("83 F8 FF 74 1D 83 3C", -10));

	scriptTickCollision.inject([] (int)
	{
		RunMissionCleanupCollisionChecks();

		scriptTickCollision.call();
	});
});
