#include "StdInc.h"
#include <MissionCleanup.h>
#include "HookCallbacks.h"

CMissionCleanup* GetMissionCleanupInstance(CMissionCleanup* instance)
{
	// make it a stack argument in case it'd be undefined behavior otherwise
	CMissionCleanup* instancePtr = instance;

	HookCallbacks::RunCallback(StringHash("mCleanup"), &instancePtr);

	return instancePtr;
}

static void __declspec(naked) CMissionCleanup__Add_pre()
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

static void __declspec(naked) CMissionCleanup__Remove_pre()
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

static HookFunction hookFunction([] ()
{
	// mission cleanup 'add script entry'
	hook::jump(0x926C60, CMissionCleanup__Add_pre);

	// mission cleanup 'remove script entry'
	hook::jump(0x926CE0, CMissionCleanup__Remove_pre);


});