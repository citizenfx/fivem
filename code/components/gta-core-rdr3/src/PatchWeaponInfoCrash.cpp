#include <StdInc.h>

#include "atArray.h"
#include "Hooking.h"
#include "Hooking.Stubs.h"

static hook::cdecl_stub<void*(uint32_t*)> getWeaponInfo([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 83 C4 ? C3 48 8B 41 ? 48 8B 40"));
});

static bool IsWeaponInfoValid(uint32_t weaponHash)
{
	// this function checks if weaponHash returns a valid CItemInfo and that it is a CWeaponInfo instance.
	void* weaponInfo = getWeaponInfo(&weaponHash);
	return weaponInfo ? true : false;
}

static HookFunction hookFunction([]
{
	// Check if CWeaponInfo from CWeaponDamageEvent is valid before continuing  
	{
		auto location = hook::get_pattern<char>("4C 8D 6F ? 45 8B 45");

		static struct : jitasm::Frontend
		{
			uintptr_t retnSuccess;
			uintptr_t retnFail;

			void Init(uintptr_t success, uintptr_t fail)
			{
				retnSuccess = success;
				retnFail = fail;
			}

			virtual void InternalMain() override
			{
				// Original Code
				lea(r13, qword_ptr[rdi + 0x58]);
				mov(r8d, dword_ptr[r13]);

				test(r8d, r8d);
				jz("Fail");

				mov(ecx, r8d);

				push(rcx);
				push(rdx);
				push(r8);
				push(r9);

				mov(rax, reinterpret_cast<uintptr_t>(&IsWeaponInfoValid));
				call(rax);

				pop(r9);
				pop(r8);
				pop(rdx);
				pop(rcx);

				test(al, al);
				jz("Fail");

				mov(rcx, retnSuccess);
				jmp(rcx);

				L("Fail");
				mov(rcx, retnFail);
				jmp(rcx);
			}
		} patchStub;

		const uintptr_t retnSuccess = (uintptr_t)location + 8;
		const uintptr_t retnFailure = (uintptr_t)hook::get_pattern("32 C0 E9 ? ? ? ? 44 38 35");

		hook::nop(location, 8);
		patchStub.Init(retnSuccess, retnFailure);
		hook::jump_rcx(location, patchStub.GetCode());
	}
});
