#include "StdInc.h"
#include "HookCallbacks.h"

void PreD3DReset()
{
	HookCallbacks::RunCallback(StringHash("d3dPreReset"), nullptr);
}

void PostD3DReset()
{
	HookCallbacks::RunCallback(StringHash("d3dPostReset"), nullptr);
}

void D3DResetFailed(HRESULT error)
{
	GlobalError("Direct3D resetting failed with error 0x%08x. This is fatal. You will probably die.", error);
}

void __declspec(naked) PreAndPostD3DReset()
{
	__asm
	{
		mov ecx, [esp + 8h]

		push ecx

		mov ecx, [eax]

		push eax

		push ecx
		call PreD3DReset
		pop ecx

		mov eax, [ecx + 40h]
		call eax

		cmp eax, 0
		jnz debugStuff

		push eax
		call PostD3DReset
		pop eax

		retn 8

debugStuff:
		push eax
		call D3DResetFailed
		add esp, 4h

		retn 8
	}
}

static HookFunction hookFunction([] ()
{
	hook::jump(0x6467B6, PreAndPostD3DReset);
});