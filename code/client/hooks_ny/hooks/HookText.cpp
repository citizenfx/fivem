#include "StdInc.h"
#include "Text.h"

const wchar_t* CText__GetCustom(const char* key)
{
	if (!key)
	{
		return nullptr;
	}

	return TheText.GetCustom(key);
}

__declspec(naked) const wchar_t* __stdcall CText__GetHook(const char*)
{
	__asm
	{
		mov eax, [esp + 4]
		push ecx
		push eax

		call CText__GetCustom

		add esp, 4h
		pop ecx

		test eax, eax
		jz returnNormal

		retn 4

returnNormal:
		sub esp, 44h
		mov eax, dword ptr ds:[0F43F40h]

		push 7B54C8h
		retn
	}
}

static HookFunction hookFunction([] ()
{
	hook::jump(0x7B54C0, CText__GetHook);
});