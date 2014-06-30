#include "StdInc.h"
#include "GameFlags.h"

static bool CTaskSimplePlayerMove__DoNetworkPlayerMove()
{
	return GameFlags::GetFlag(GameFlag::NetworkWalkMode);
}

static void TapSprintIfNeeded(char* esi)
{
	if (CTaskSimplePlayerMove__DoNetworkPlayerMove())
	{
		esi[12938] = 1;
		esi[12941] = 1;
	}
}

static void __declspec(naked) TapSprintIfNeededStub()
{
	__asm
	{
		push esi
		call TapSprintIfNeeded
		add esp, 4h

		retn
	}
}

static HookFunction hookFunction([] ()
{
	// non-sprint state
	hook::call(0x9AF911, CTaskSimplePlayerMove__DoNetworkPlayerMove);
	hook::call(0x9AF937, CTaskSimplePlayerMove__DoNetworkPlayerMove);
	hook::call(0x9AF9EF, CTaskSimplePlayerMove__DoNetworkPlayerMove);

	// alternate mode
	hook::call(0x9B574F, CTaskSimplePlayerMove__DoNetworkPlayerMove);

	// move blend, sprinting behavior
	hook::call(0xA35EF5, CTaskSimplePlayerMove__DoNetworkPlayerMove);

	// jump over some 'key_sprint' raw keyboard check which happens to define automatic tapping for the sprint button if lshift is used
	//hook::put<uint8_t>(0x838779, 0xEB);

	// the same as above, but dynamic
	hook::nop(0x83877B, 14);
	hook::call(0x83877B, TapSprintIfNeededStub);
});