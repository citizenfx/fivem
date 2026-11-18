#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

#include <Streaming.h>

static bool ShouldBindHDTxd(uint32_t hdTxdHandle, uint32_t targetHandle)
{
	// original check
	if (((hdTxdHandle ^ targetHandle) & 0xFFFF0000) == 0)
	{
		return true;
	}

	// unregistered
	if (hdTxdHandle == 0 || targetHandle == 0)
	{
		return false;
	}

	// the first 3 collections are raw streamers
	return streaming::GetCollectionIndex(hdTxdHandle) < 3 || streaming::GetCollectionIndex(targetHandle) < 3;
}

static HookFunction hookFunction([]()
{
	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			mov(edx, ebx);

			sub(rsp, 0x28);
			mov(rax, (uintptr_t)ShouldBindHDTxd);
			call(rax);
			add(rsp, 0x28);

			test(al, al);
			ret();
		}
	} compareStub;

	auto location = hook::get_pattern<char>("33 C3 A9 00 00 FF FF 75 04 B0 01");
	hook::put<uint16_t>(location, 0xC189); // mov ecx, eax
	hook::call(location + 2, compareStub.GetCode());
	hook::put<uint16_t>(location + 7, 0x0474); // jz fail
});
