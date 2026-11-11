#include <StdInc.h>

#include <CoreConsole.h>
#include <Hooking.h>
#include <jitasm.h>

//
// CTaskAimGun::CalculateFiringVector can fire from the aiming camera. GFF_FireFromMuzzle instead probes the
// camera target and uses the weapon muzzle as the shot origin. This convar enables that path for the local
// player without changing the task's flags or the normal weapon firing call. When disabled, the original
// GFF_FireFromMuzzle check and WEAPON_RAYMINIGUN exception still apply.
//

static bool g_enableFireFromMuzzle = false;

static HookFunction hookFunction([]()
{
	static ConVar<bool> enableFireFromMuzzle("game_enableFireFromMuzzle", ConVar_Replicated, false, &g_enableFireFromMuzzle);

	auto location = hook::get_pattern<char>("45 84 B5 B0 00 00 00 0F 85 ? ? ? ? 49 8B 47 40 81 78 10 67 1F 2D B6 0F 84");

	static struct : jitasm::Frontend
	{
		uintptr_t m_returnAddress;

		void Init(uintptr_t returnAddress)
		{
			m_returnAddress = returnAddress;
		}

		void InternalMain() override
		{
			// rax is overwritten on both paths after the test, so it can be used here
			mov(rax, reinterpret_cast<uintptr_t>(&g_enableFireFromMuzzle));
			cmp(byte_ptr[rax], 0);
			jne("done");

			// original GFF_FireFromMuzzle test, with r14b holding the bit-0 mask
			test(byte_ptr[r13 + 0xB0], r14b);

			L("done");
			mov(rax, m_returnAddress);
			jmp(rax);
		}
	} stub;

	// resume at the original conditional jump
	stub.Init(reinterpret_cast<uintptr_t>(location + 7));
	hook::nop(location, 7);
	hook::jump(location, stub.GetCode());
});
