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
		uintptr_t m_origBikeJumpBoolLoc{ 0 };
		uintptr_t m_defaultPathLoc{ 0 };
		uintptr_t m_continueLoc{ 0 };
		uintptr_t m_earlyOutLoc{ 0 };
		uint8_t m_impulseVarStackOffset{ 0 };

		float m_origJumpImpulseVec[4]{ 0.0f, 0.0f, 1.0f, 0.0f };
		float m_minBunnyHopTime{ 0.0f };

		void Init(uintptr_t origBikeJumpBoolLoc, uintptr_t defaultPathLoc, uintptr_t continueLoc,
		uintptr_t earlyOutLoc, uint8_t impulseVarStackOffset)
		{
			m_origBikeJumpBoolLoc = origBikeJumpBoolLoc;
			m_defaultPathLoc = defaultPathLoc;
			m_continueLoc = continueLoc;
			m_earlyOutLoc = earlyOutLoc;
			m_impulseVarStackOffset = impulseVarStackOffset;
		}

		void InternalMain() override
		{
			constexpr uintptr_t kVec3ZWOffset{ sizeof(float) * 2 };

			auto origJumpImpulseLoc = reinterpret_cast<uintptr_t>(m_origJumpImpulseVec);
			auto minBunnyHopTimeLoc = reinterpret_cast<uintptr_t>(&m_minBunnyHopTime);

			mov(rax, minBunnyHopTimeLoc);
			comiss(xmm15, dword_ptr[rax]);
			jbe("earlyOut");

			mov(rax, m_origBikeJumpBoolLoc);
			mov(al, byte_ptr[rax]);
			test(al, al);
			jz("useDefaultPhysics");

			L("useOriginalPhysics");
			// Load the original jump impulse.
			mov(rax, origJumpImpulseLoc);
			movlps(xmm9, qword_ptr[rax]); // Impulse XY -> XMM9.
			movlps(xmm10, qword_ptr[rax + kVec3ZWOffset]); // Impulse ZW -> XMM10.

			// Store the impulse value to the proper variable on the stack.
			movlps(qword_ptr[rsp + m_impulseVarStackOffset], xmm9); // XMM9 -> impulse var XY.
			movlps(qword_ptr[rsp + m_impulseVarStackOffset + kVec3ZWOffset], xmm10); // XMM10 -> impulse var ZW.

			movsx(edx, byte_ptr[rbx + 0x0CA1]);
			movsx(r9d, byte_ptr[rbx + 0x0CA0]);
			mov(r8d, edx);

			// Continue execution with the new impulse.
			mov(rcx, m_continueLoc);
			jmp(rcx);

			L("useDefaultPhysics");
			mov(rcx, m_defaultPathLoc);
			jmp(rcx);

			L("earlyOut");
			mov(rcx, m_earlyOutLoc);
			jmp(rcx);
		}
	} patchStub;

	auto patchLoc = hook::get_pattern<char>("44 0F 2F 3D ? ? ? ? 0F 86 ? ? ? ? 0F 28 7B");

	auto useOrigBikeJumpBoolPtr = reinterpret_cast<uintptr_t>(&g_useOrigBikeJump);

	auto defaultPathLoc = hook::get_pattern<char>("44 0F 2F 3D ? ? ? ? 0F 86 ? ? ? ? 0F 28 7B", 0x0E);
	auto defaultPathLocPtr = reinterpret_cast<uintptr_t>(defaultPathLoc);

	auto continueLoc = hook::get_pattern<char>("41 3B D1 73 ? 48 0F BE C2 48 8B 8C C3 ? ? ? ? EB ? 33 C9 48 85 C9 0F 84 ? ? ? ? 45 3B C1 73 ? 48 0F BE C2 48 8B 8C C3 ? ? ? ? EB ? 33 C9 8A 81");
	auto continueLocPtr = reinterpret_cast<uintptr_t>(continueLoc);

	auto earlyOutLoc = hook::get_pattern<char>("4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 49 8B 73 ? 41 0F 28 73 ? 41 0F 28 7B ? 45 0F 28 43 ? 45 0F 28 4B ? 45 0F 28 53 ? 45 0F 28 5B ? 45 0F 28 63 ? 45 0F 28 6B ? 45 0F 28 B3 ? ? ? ? 45 0F 28 BB ? ? ? ? 49 8B E3 5D");
	auto earlyOutLocPtr = reinterpret_cast<uintptr_t>(earlyOutLoc);

	auto varOffset = hook::get_pattern("F3 0F 10 4C 24 ? F3 0F 10 54 24 ? 0F BE 93", 5);
	uint8_t impulseVarOffset = *static_cast<uint8_t*>(varOffset);

	patchStub.Init(useOrigBikeJumpBoolPtr, defaultPathLocPtr, continueLocPtr, earlyOutLocPtr, impulseVarOffset);

	hook::jump(patchLoc, patchStub.GetCode());
});
