#include <StdInc.h>

#include <CoreConsole.h>
#include <CrossBuildRuntime.h>
#include <Hooking.h>
#include <jitasm.h>

static bool g_useOrigSirenToggle = false;

static HookFunction hookFunction([]()
{
	static ConVar<bool> originalSirenToggle("game_originalSirenToggle", ConVar_Replicated, false, &g_useOrigSirenToggle);

	if (!xbr::IsGameBuildOrGreater<3407>())
	{
		return;
	}

	static struct : jitasm::Frontend
	{
		uintptr_t m_boolLoc{ 0 };
		uintptr_t m_gameInProgressLoc{ 0 };
		uintptr_t m_jmpBackLoc{ 0 };

		void Init(uintptr_t boolLoc, uintptr_t gameInProgressLoc, uintptr_t jmpBackLoc)
		{
			m_boolLoc = boolLoc;
			m_gameInProgressLoc = gameInProgressLoc;
			m_jmpBackLoc = jmpBackLoc;
		}

		void InternalMain() override
		{
			mov(rax, m_boolLoc);
			cmp(byte_ptr[rax], 0);
			jz("original");

			cmp(r12b, r12b);
			jmp("done");

			L("original");
			mov(rax, m_gameInProgressLoc);
			cmp(byte_ptr[rax], r12b);

			L("done");
			mov(rax, m_jmpBackLoc);
			jmp(rax);
		}
	} patchStub;

	auto patchLoc = hook::get_pattern<char>("44 38 25 ? ? ? ? 74 2A F6 87 6B 09 00 00 02 74 21 8A 87 85 09 00 00 A8 40 74 17 41 84 C7 75 12 F6 87 86 09 00 00 20 75 09");

	auto gameInProgressLoc = hook::get_address<uint8_t*>(patchLoc, 3, 7);
	auto jmpBackLoc = reinterpret_cast<uintptr_t>(patchLoc + 7);

	patchStub.Init(reinterpret_cast<uintptr_t>(&g_useOrigSirenToggle), reinterpret_cast<uintptr_t>(gameInProgressLoc), jmpBackLoc);

	hook::nop(patchLoc, 7);
	hook::jump(patchLoc, patchStub.GetCode());
});
