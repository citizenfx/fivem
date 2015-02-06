/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include "scrThread.h"
#include "scrEngine.h"

static uint32_t gtaThreadVTable;

bool IsSafeScriptVTable(uint32_t vTable)
{
	return (vTable != gtaThreadVTable);
}

static rage::eThreadState ScriptTickDo(rage::scrThread* thread, int time)
{
	bool allowed = false;

	rage::scrEngine::CheckNativeScriptAllowed(allowed);

	if (allowed || IsSafeScriptVTable(*(uint32_t*)thread))
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