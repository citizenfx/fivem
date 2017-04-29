/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "DrawCommands.h"
#include "Hooking.h"

fwEvent<> OnGrcCreateDevice;
fwEvent<> OnGrcBeginScene;
fwEvent<> OnGrcEndScene;

static void InvokeEndSceneCB()
{
	//HookCallbacks::RunCallback(StringHash("renderCB"), "endScene");
	OnGrcEndScene();
	OnPostFrontendRender();
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

static void __stdcall InvokeCreateCB(void*, void*)
{
	OnGrcCreateDevice();
}

static HookFunction hookFunction([] ()
{
	static hook::inject_call<void, int> beginSceneCB(0x633403);
	beginSceneCB.inject([] (int)
	{
		beginSceneCB.call();

		OnGrcBeginScene();
	});


	// end scene callback dc
	//hook::put(0x796B9E, InvokeEndSceneCBStub);

	// same, for during loading text
	hook::call(0x7BD74D, InvokeEndSceneCBStub);

	// device creation
	hook::jump(0xD3033C, InvokeCreateCB);

	// frontend render phase
	//hook::put(0xE9F1AC, InvokeFrontendCBStub);

	// in-menu check for renderphasefrontend
	//*(BYTE*)0x43AF21 = 0xEB;
});