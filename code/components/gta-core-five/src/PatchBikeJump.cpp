#include <StdInc.h>

#include <CoreConsole.h>
#include <CrossBuildRuntime.h>
#include <Hooking.h>
#include <jitasm.h>

//
// Starting at b3095 the physics for bike bunny hopping were changed, calculating the jump impulse
// with respect to the bike's forward velocity. This patch adds a convar that restores
// the original behavior - using a unit Z vector as the impulse, without any modifications.
//
// History:
//	* b2944 and earlier: original bunny hopping behavior, impulse is directly up on Z axis
//	* b3095: Bunny hopping behavior changed, impulse now takes bike's forward momentum into account
//	* b3407: Refinements to changes made in b3095
//	

static bool g_useOrigBikeJump = false;

static HookFunction hookFunction([]()
{
	static ConVar<bool> originalBikeJump("game_originalBikeJump", ConVar_Replicated, false, &g_useOrigBikeJump);

	// This patch only applies to b3407 and above.
	if (!xbr::IsGameBuildOrGreater<3407>())
	{
		return;
	}

	static struct : jitasm::Frontend
	{
		uintptr_t m_boolLoc{ 0 };
		uintptr_t m_jmpBackLoc{ 0 };
		uint8_t m_impulseOffX{ 0 };

		void Init(uintptr_t boolLoc, uintptr_t jmpBackLoc, uint8_t impulseOffX)
		{
			m_boolLoc = boolLoc;
			m_jmpBackLoc = jmpBackLoc;
			m_impulseOffX = impulseOffX;
		}

		void InternalMain() override
		{
			const uint8_t offX = m_impulseOffX;
			const uint8_t offY = static_cast<uint8_t>(offX + sizeof(float));
			const uint8_t offZ = static_cast<uint8_t>(offX + sizeof(float) * 2);

			// Reproduce the original stores of the computed impulse
			movss(dword_ptr[rsp + offX], xmm1); // impulse.X
			movss(dword_ptr[rsp + offY], xmm2); // impulse.Y
			movss(dword_ptr[rsp + offZ], xmm3); // impulse.Z

			mov(rax, m_boolLoc);
			cmp(byte_ptr[rax], 0);
			jz("done");

			// Restore the original world space unit Z impulse: (0, 0, 1)
			mov(dword_ptr[rsp + offX], 0x00000000);
			mov(dword_ptr[rsp + offY], 0x00000000);
			mov(dword_ptr[rsp + offZ], 0x3F800000); // 1.0f

			L("done");
			mov(rax, m_jmpBackLoc);
			jmp(rax);
		}
	} patchStub;

	auto patchLoc = hook::get_pattern<char>("F3 0F 11 4C 24 ? F3 0F 11 54 24 ? F3 0F 11 5C 24 ? 41 3B D1");

	// The impulse X component stack offset is the imm8 of the first store.
	uint8_t impulseOffX = *reinterpret_cast<uint8_t*>(patchLoc + 5);

	// Resume right after the three reproduced stores (3 * 6 = 18 bytes)
	auto jmpBackLoc = reinterpret_cast<uintptr_t>(patchLoc + 18);

	patchStub.Init(reinterpret_cast<uintptr_t>(&g_useOrigBikeJump), jmpBackLoc, impulseOffX);

	hook::jump(patchLoc, patchStub.GetCode());
});
