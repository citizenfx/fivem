#include <StdInc.h>
#include "Hooking.Patterns.h"

//
// The artificial lights state crash is caused by vehicles that use "CCustomShaderEffectStandardVehicle"
// instead of "CCustomShaderEffectVehicle" (such as "keelboat"). The "StandardVehicle" one doesn't support
// emissive settings overrides, and the class is much smaller than the non-Standard one, so the game is
// trying to write these values out of bounds, causing a crash. Everything looks like a simple oversight,
// we're fixing it by comparing the vehicle's custom shader effect vtable address against the vtable address
// of "CCustomShaderEffectVehicle". Why use an asm-patch instead of just hooking the whole function and
// checking for the shader beforehand? Because there are many other things happening in this specific
// function that don't expect this specific custom shader effect to be used explicitly. To avoid any kind
// of regressions, we only patch this specific place, allowing the game to execute this function on all vehicles.
//

static uintptr_t g_customShaderEffectVehicleVtbl;
static bool* g_artificialLightsState;

static bool CanOverrideLightsDataOnVehicle(char* vehicle)
{
	if (!*g_artificialLightsState) // Original check
	{
		return false;
	}

	if (const auto drawHandler = *reinterpret_cast<char**>(vehicle + 0x28))
	{
		if (const auto customShaderEffect = *reinterpret_cast<void**>(drawHandler + 0x18))
		{
			// Comparing vtable addresses, if the vehicle's custom shader effect is
			// an instance of "CCustomShaderEffectVehicle", we can continue safely.
			return *reinterpret_cast<uintptr_t*>(customShaderEffect) == g_customShaderEffectVehicleVtbl;
		}
	}

	return false;
}

static HookFunction hookFunction([]()
{
	static struct : jitasm::Frontend
	{
		intptr_t retSuccess;
		intptr_t retFail;

		void Init(intptr_t success, intptr_t fail)
		{
			this->retSuccess = success;
			this->retFail = fail;
		}

		virtual void InternalMain() override
		{
			push(rcx);
			sub(rsp, 0x28);

			mov(rcx, rdi); // pass CVehicle* as the first argument

			mov(rax, reinterpret_cast<intptr_t>(CanOverrideLightsDataOnVehicle));
			call(rax);

			add(rsp, 0x28);
			pop(rcx);

			test(al, al);
			jz("fail");

			mov(rax, retSuccess);
			jmp(rax);

			L("fail");

			mov(rax, retFail);
			jmp(rax);
		}
	} patchStub;

	{
		auto location = hook::get_pattern<char>("44 38 3D ? ? ? ? 0F 84 ? ? ? ? 45 0F 2E F5");
		g_artificialLightsState = hook::get_address<bool*>(location + 3);

		// Skipping "cmp" and "jz" instructions (7 + 6 bytes).
		const auto successPtr = reinterpret_cast<intptr_t>(location) + 13;

		// Using offset from "jz" to find where we should bail out.
		const auto failPtr = successPtr + *reinterpret_cast<uint32_t*>(location + 9);

		patchStub.Init(successPtr, failPtr);

		hook::nop(location, 13);
		hook::jump(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("41 B8 03 00 00 00 48 8B D9 E8", 21);	
		g_customShaderEffectVehicleVtbl = hook::get_address<uintptr_t>(location);
	}
});
