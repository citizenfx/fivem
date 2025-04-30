#include "StdInc.h"

#include "CrossBuildRuntime.h"
#include "Hooking.h"

#include <jitasm.h>

static uint32_t CWeaponInfo_ClassId_hash = HashString("CWeaponInfo");

static bool (*VerifyCWeaponInfoPtr)(void*);

using CWeaponInfo_GetClassId_t = uint32_t(__fastcall*)(void*, uint32_t*);
using CWeaponInfo_GetClassId_Legacy_t = uint32_t*(__fastcall*)(void*, uint32_t*);

template<int GameBuild>
static bool VerifyCWeaponInfo(void* CWeaponInfo)
{
	const auto vtable = *(void***)CWeaponInfo;

	uint32_t result;

	if constexpr (GameBuild >= 2802)
	{
		const auto GetClassId = (CWeaponInfo_GetClassId_t)(vtable[2]);
		result = static_cast<uint32_t>(uintptr_t(GetClassId(CWeaponInfo, nullptr)));
	}
	else
	{
		uint32_t tmp;
		const auto GetClassId = (CWeaponInfo_GetClassId_Legacy_t)(vtable[2]);
		result = *GetClassId(CWeaponInfo, &tmp);
	}

	return CWeaponInfo_ClassId_hash == result;
}

static HookFunction hookFunction([]()
{
	if (xbr::IsGameBuildOrGreater<3258>())
	{
		return;
	}

	static struct : jitasm::Frontend
	{
		intptr_t RetSuccess;
		intptr_t RetFail;

		void Init(const intptr_t retSuccess, const intptr_t retFail)
		{
			this->RetSuccess = retSuccess;
			this->RetFail = retFail;
		}

		void InternalMain() override
		{
			// Original check with changed redirection to early zero return
			test(rcx, rcx);
			jz("fail");

			// Preserve registers
			push(rcx);
			push(rdx);
			push(r8);
			push(r9);

			// Shadow space + stack alignment
			sub(rsp, 0x28);

			// Injected check to verify CWeaponInfo has a valid class instance
			mov(rax, reinterpret_cast<uintptr_t>(VerifyCWeaponInfoPtr));
			call(rax);

			add(rsp, 0x28);

			// Restore registers
			pop(r9);
			pop(r8);
			pop(rdx);
			pop(rcx);

			test(eax, eax);
			jz("fail"); // Early zero return

			mov(rax, RetSuccess);
			jmp(rax);

			L("fail");
			mov(rax, RetFail);
			jmp(rax);
		}
	} patchStub;

	const auto location = hook::get_pattern("48 85 C9 74 ? F3 0F 10 81 ? ? ? ? EB ? 0F 57 C0");

	const auto retSuccess = (intptr_t)location + 5;

	const auto retFail = (intptr_t)hook::get_pattern("32 C0 4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 41 0F 28 73 ? 41 0F 28 7B ? 45 0F 28 43 ? 45 0F 28 4B ? 45 0F 28 53 ? 45 0F 28 5B ? 45 0F 28 63 ? 45 0F 28 6B ? 45 0F 28 B3 ? ? ? ? 45 0F 28 BB ? ? ? ? 49 8B E3 41 5F 41 5E 41 5D 41 5C 5F 5E 5D C3 BF");

	if (xbr::IsGameBuildOrGreater<2802>())
	{
		VerifyCWeaponInfoPtr = &VerifyCWeaponInfo<2802>;
	}
	else
	{
		VerifyCWeaponInfoPtr = &VerifyCWeaponInfo<1604>;
	}

	patchStub.Init(retSuccess, retFail);

	hook::nop(location, 5);
	hook::jump(location, patchStub.GetCode());
});
