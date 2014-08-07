#include "StdInc.h"

DWORD hashHookRet = 0x5B1C35;

DWORD HashHookForceHash(const char* string)
{
	if (!_strnicmp(string, "hash:", 5))
	{
		return strtoul(&string[5], nullptr, 0);
	}

	return 0;
}

void __declspec(naked) HashHook()
{
	__asm
	{
		mov eax, [esp + 4]
		push eax

		call HashHookForceHash

		add esp, 4h

		test eax, eax
		jz continueStuff

		retn

continueStuff:
		mov ecx, [esp + 8]
		push esi

		jmp hashHookRet
	}
}

static HookFunction hookFunction([] ()
{
	hook::jump(0x5B1C30, HashHook);
});