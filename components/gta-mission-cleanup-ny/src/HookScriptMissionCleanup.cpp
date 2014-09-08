#include "StdInc.h"
#include <Hooking.h>
#include <MissionCleanup.h>

CMissionCleanup* GetMissionCleanupInstance(CMissionCleanup* instance)
{
	// make it a stack argument in case it'd be undefined behavior otherwise
	CMissionCleanup* instancePtr = instance;

	//HookCallbacks::RunCallback(StringHash("mCleanup"), &instancePtr);
	CMissionCleanup::OnQueryMissionCleanup(instancePtr);

	return instancePtr;
}

static void __declspec(naked) CMissionCleanup__AddEntityToList_pre()
{
	__asm
	{
		push ecx
		call GetMissionCleanupInstance
		add esp, 4h

		// overwrite ecx with eax
		mov ecx, eax

		// and return as usual
		push ebx
		mov ebx, [esp + 4 + 0Ch]

		push 926C65h
		retn
	}
}

static void __declspec(naked) CMissionCleanup__RemoveEntityFromList_pre()
{
	__asm
	{
		push ecx
		call GetMissionCleanupInstance
		add esp, 4h

		// overwrite ecx with eax
		mov ecx, eax

		// and return as usual
		push ebx
		mov bl, byte ptr [esp + 4 + 8]

		push 926CE5h
		retn
	}
}

static void __declspec(naked) CMissionCleanup__CheckIfCollisionHasLoadedForMissionObjects_pre()
{
	__asm
	{
		push ecx
		call GetMissionCleanupInstance
		add esp, 4h

		mov ecx, eax

		sub esp, 8
		push ebx
		push ebp

		push 9282D5h
		retn
	}
}

static void RunMissionCleanupCollisionChecks()
{
	CMissionCleanup::OnCheckCollision();
}

fwEvent<CMissionCleanup*&> CMissionCleanup::OnQueryMissionCleanup;
fwEvent<> CMissionCleanup::OnCheckCollision;

static HookFunction hookFunction([] ()
{
	// mission cleanup 'add script entry'
	hook::jump(0x926C60, CMissionCleanup__AddEntityToList_pre);

	// mission cleanup 'remove script entry'
	hook::jump(0x926CE0, CMissionCleanup__RemoveEntityFromList_pre);

	// mission cleanup 'check collision'
	hook::jump(0x9282D0, CMissionCleanup__CheckIfCollisionHasLoadedForMissionObjects_pre);

	// per-tick mission cleanup collision check
	static hook::inject_call<void, int> scriptTickCollision(0x80A04C);

	scriptTickCollision.inject([] (int)
	{
		RunMissionCleanupCollisionChecks();

		scriptTickCollision.call();
	});
});