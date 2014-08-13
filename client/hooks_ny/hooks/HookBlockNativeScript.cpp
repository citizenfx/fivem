#include "StdInc.h"
#include "scrThread.h"

static uint32_t gtaThreadVTable;

bool IsSafeScriptVTable(uint32_t vTable)
{
	return (vTable != gtaThreadVTable);
}

static rage::eThreadState ScriptTickDo(rage::scrThread* thread, int time)
{
	if (IsSafeScriptVTable(*(uint32_t*)thread))
	{
		thread->Tick(time);
	}

	return thread->GetContext()->State;
}

static void __declspec(naked) ScriptTick()
{
	__asm
	{
		push edi
		push ecx
		call ScriptTickDo
		add esp, 8h

		retn
	}
}

static HookFunction hookFunction([] ()
{
	hook::call(0x5A6040, ScriptTick);
	hook::nop(0x5A6045, 1);

	gtaThreadVTable = 0xDAE9C4;
});