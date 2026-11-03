#include <StdInc.h>

#include <CoreConsole.h>
#include <CrossBuildRuntime.h>
#include <Hooking.h>
#include <jitasm.h>

static bool g_useOrigDriftTyres = false;

static HookFunction hookFunction([]()
{
	static ConVar<bool> originalDriftTyres("game_originalDriftTyres", ConVar_Replicated, false, &g_useOrigDriftTyres);

	if (!xbr::IsGameBuildOrGreater<3095>())
	{
		return;
	}

	static struct : jitasm::Frontend
	{
		uintptr_t m_boolLoc{ 0 };
		uintptr_t m_jmpBackLoc{ 0 };

		void Init(uintptr_t boolLoc, uintptr_t jmpBackLoc)
		{
			m_boolLoc = boolLoc;
			m_jmpBackLoc = jmpBackLoc;
		}

		void InternalMain() override
		{
			mov(rax, m_boolLoc);
			cmp(byte_ptr[rax], 0);
			jnz("original");

			mulss(xmm15, xmm0);
			jmp("done");

			L("original");
			mulss(xmm9, xmm0);

			L("done");
			mov(rax, m_jmpBackLoc);
			jmp(rax);
		}
	} patchStub;

	auto patchLoc = hook::get_pattern<char>("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 0F 2F BB ? ? ? ? 77 0A F3 0F 10 05 ? ? ? ? EB 08 F3 0F 10 05 ? ? ? ? F3 44 0F 59 F8", 40);

	patchStub.Init(reinterpret_cast<uintptr_t>(&g_useOrigDriftTyres), reinterpret_cast<uintptr_t>(patchLoc + 5));

	hook::jump(patchLoc, patchStub.GetCode());
});
