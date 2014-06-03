#include "StdInc.h"
#include "HookCallbacks.h"

static void InvokeFrontendCB()
{
	HookCallbacks::RunCallback(StringHash("renderCB"), "frontend");
}

static void WRAPPER AllocDCStub() { EAXJMP(0x7BDD80); }

static void __declspec(naked) InvokeFrontendCBStub()
{
	__asm
	{
		mov eax, 44CCD0h
		call eax

		jmp InvokeFrontendCB
	}
}

static void InvokeEndSceneCB()
{
	HookCallbacks::RunCallback(StringHash("renderCB"), "endScene");
}

static void __declspec(naked) InvokeEndSceneCBStub()
{
	__asm
	{
		call InvokeEndSceneCB

		push 796920h
		retn
	}
}



static HookFunction hookFunction([] ()
{
	// end scene callback dc
	hook::put(0x796B9E, InvokeEndSceneCBStub);

	// same, for during loading text
	hook::call(0x7BD74D, InvokeEndSceneCBStub);

	// frontend render phase
	//hook::put(0xE9F1AC, InvokeFrontendCBStub);

	// in-menu check for renderphasefrontend
	//*(BYTE*)0x43AF21 = 0xEB;
});