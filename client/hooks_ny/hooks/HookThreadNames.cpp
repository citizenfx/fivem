#include "StdInc.h"

static void __declspec(naked) PassNewArgument()
{
	__asm
	{
		mov [esi], ecx
		mov [esi + 4], edx

		mov ecx, [esp + 0Ch + 14h]
		mov [esi + 10h], ecx
		retn
	}
}

static LPTHREAD_START_ROUTINE origStart;

static DWORD WINAPI NewThreadStart(LPVOID arg)
{
	SetThreadName(GetCurrentThreadId(), *(((char**)arg) + 4));

	return origStart(arg);
}

static HookFunction hookFunction([] ()
{
	hook::call(0x5A8814, PassNewArgument);

	origStart = *(LPTHREAD_START_ROUTINE*)(0x5A882D);
	hook::put(0x5A882D, NewThreadStart);
});