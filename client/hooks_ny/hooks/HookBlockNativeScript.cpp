#include "StdInc.h"
#include "scrThread.h"

static uint32_t gtaThreadVTable;

bool IsSafeScriptVTable(uint32_t vTable)
{
	return (vTable != gtaThreadVTable);
}

DEFINE_INJECT_HOOK(preTickCheck, 0x5A6040)
{
	rage::scrThread* thread = reinterpret_cast<rage::scrThread*>(Ecx());

	if (IsSafeScriptVTable(Edx()))
	{
		thread->Tick(Edi());
	}

	Eax(thread->GetContext()->State);

	return DoNowt();
}

static HookFunction hookFunction([] ()
{
	preTickCheck.injectCall();
	hook::nop(0x5A6045, 1);

	gtaThreadVTable = 0xDAE9C4;
});