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
	// ignore startup/network_startup
	hook::put<uint8_t>(hook::get_pattern("46 83 FE 20 72 ? 80 7D 08 00", 17), 0xEB);

	auto loc = hook::get_pattern<char>("83 79 04 00 74 ? 8B 01 57 FF 50 0C");
	hook::nop(loc + 6, 6);
	hook::call(loc + 6, ScriptTick);

	gtaThreadVTable = *hook::get_pattern<uint32_t>("C7 86 A8 00 00 00 00 00 00 00 8B C6", -9);
});
