#include "StdInc.h"
#include "CrossBuildRuntime.h"
#include <Hooking.h>
#include <jitasm.h>

constexpr int kMaxBlipLegend = 256;

static HookFunction hookFunction([]()
{
	if (!xbr::IsGameBuildOrGreater<2372>()) return;

	static struct : jitasm::Frontend
	{
		intptr_t toJae, toNext;

		void Init(intptr_t jTarget, intptr_t nTarget)
		{
			toJae = jTarget;
			toNext = nTarget;
		}

		void InternalMain() override
		{
			cmp(r8w, kMaxBlipLegend);
			jae("maxBlipReached");
			mov(rax, toNext);
			jmp(rax);
			L("maxBlipReached");
			mov(rax, toJae);
			jmp(rax);
		}
	} stub;

	const char* pattern = xbr::IsGameBuildOrGreater<3407>()
		? "66 44 3B C0 73 ? 44 8B 74 24 30 EB"
		: "66 41 83 F8 ? 73 ? 44 8B 74 24 30 EB";

	const int jaeOffset = xbr::IsGameBuildOrGreater<3407>() ? 4 : 5;

	auto location = hook::get_pattern<char>(pattern);
	auto jae = location + jaeOffset;
	const int8_t disp = *(int8_t*)(jae + 1);
	const intptr_t toJae  = (intptr_t)(jae + 2 + disp);
	const intptr_t toNext = (intptr_t)(jae + 2);

	stub.Init(toJae, toNext);

	hook::nop(location, jaeOffset + 2);
	hook::jump(location, stub.GetCode());
});
