#include <StdInc.h>

#include <CoreConsole.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <jitasm.h>

static bool g_DisablePassiveMode = false;

static HookFunction hookFunction([]
{
	static ConVar<bool> enableVehicleHijackFix("game_disablePassiveMode", ConVar_Replicated, false, &g_DisablePassiveMode);

	auto patchAllowedToDamagePassiveFlagLocation = hook::get_pattern("C1 E8 ? A8 ? 75 ? C1 E9 ? F6 C1 ? 75 ? 48 8B D3");
	auto patchAllowedToDamageSkipLocation = hook::get_pattern("32 C0 E9 ? ? ? ? F7 83");

	static struct : jitasm::Frontend
	{
		uintptr_t m_ContinueAddress;
		uintptr_t m_SkipAddress;

		void Init(uintptr_t continueAddress, uintptr_t skipAddress)
		{
			m_ContinueAddress = continueAddress;
			m_SkipAddress = skipAddress;
		}

		void InternalMain() override
		{
			shr(eax, 7);
			test(al, 1);
			jz("Continue");

			mov(r11, reinterpret_cast<uintptr_t>(&g_DisablePassiveMode));
			cmp(byte_ptr[r11], 0);
			jne("Continue");

			mov(r11, m_SkipAddress);
			jmp(r11);

			L("Continue");
			mov(r11, m_ContinueAddress);
			jmp(r11);
		}
	} patchStub;

	patchStub.Init((uintptr_t)patchAllowedToDamagePassiveFlagLocation + 7, (uintptr_t)patchAllowedToDamageSkipLocation);

	hook::nop(patchAllowedToDamagePassiveFlagLocation, 7);
	hook::jump_rcx(patchAllowedToDamagePassiveFlagLocation, patchStub.GetCode());
});
