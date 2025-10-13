#include "StdInc.h"
#include <jitasm.h>
#include "Hooking.Patterns.h"

static HookFunction hookFunction([]()
{
	// CDecalManager::AddWeaponImpact
	// This function is responsible for spawning visual effects (bullet decals, sparks, etc)
	// based on weapon impact data such as world position, direction, surface material.
	//
	// However, the function assumes that the provided VfxWeaponInfo pointer is always valid. In cases where
	// the input material or weapon effect group does not have a matching VFX configuration. Malicious actors
	// may exploit this by sending invalid network data that causes other clients to trigger decal creation
	// with unregistered or unsupported materials or weapon groups, leading to a crash when attempting to
	// read a null VfxWeaponInfo pointer.
	
	auto location = hook::get_pattern<char>("F3 0F 10 4E ? 8B 05");
	auto failLocation = hook::get_pattern("38 9D ? ? ? ? 74 ? 4C 8D 4C 24 ? 4C 8D 44 24");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}
		
		virtual void InternalMain() override
		{
			test(rsi, rsi);
			jz("fail");

			movss(xmm1, dword_ptr[rsi+0x28]);
			mov(rax, successLocation);
			jmp(rax);

			L("fail");
			mov(rax, failLocation);
			jmp(rax);
		}
	} stub;
	
	assert(((intptr_t)failLocation - (intptr_t)location) < 2000);
	stub.Init((uintptr_t)location + 5, (uintptr_t)failLocation);
	
	hook::nop(location, 5);
	hook::jump(location, stub.GetCode());
});