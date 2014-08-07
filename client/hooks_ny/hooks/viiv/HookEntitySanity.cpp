#include "StdInc.h"

static DWORD entityCheck2HookDont = 0xB2B407;
static DWORD entityCheck2HookDo = 0xB2B269;

void __declspec(naked) EntityCheck2Hook()
{
	__asm
	{
		cmp [esi + 3Ch], eax
		jz dont

		cmp word ptr [esi + 2Eh], 0FFFFh
		je dont

		jmp entityCheck2HookDo

dont:
		jmp entityCheck2HookDont
	}
}

DWORD entityCheck3Dont = 0x9E7073;

void __declspec(naked) EntityCheck3Hook()
{
	__asm
	{
		cmp ax, 0FFFFh
		je dont

		cmp word ptr [edi + 52h], 0FFFFh
		retn

dont:
		mov eax, entityCheck3Dont
		mov dword ptr [esp], eax
		retn
	}
}

bool __fastcall SomeEntityCheckHook(char* entity)
{
	if (*(uint16_t*)(entity + 46) == 0xFFFF)
	{
		return true;
	}

	return ((bool(__fastcall*)(char*))(0x9E7AB0))(entity);
}

static RuntimeHookFunction esrhf("entity_sanity", [] ()
{
	hook::call(0x7D7849, SomeEntityCheckHook);

	// another crash fix for render list entities
	hook::jump(0xB2B260, EntityCheck2Hook);

	// and yet another one
	hook::call(0x9E7055, EntityCheck3Hook);
});