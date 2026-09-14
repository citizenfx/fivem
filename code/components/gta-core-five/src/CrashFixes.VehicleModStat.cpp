/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <CrossBuildRuntime.h>

struct VehicleModStatStub : jitasm::Frontend
{
	uintptr_t returnAddress;

	void Init(uintptr_t location)
	{
		returnAddress = location + 5;
	}

	void InternalMain() override
	{
		// The mod state is embedded at drawHandler + 0x40. A missing draw
		// handler therefore reaches this function as 0x40, not nullptr.
		cmp(rcx, 0x40);
		je("noModifier");

		// Replay the overwritten prologue and resume the original lookup.
		// RAX is scratch here; the original lookup overwrites it before use.
		mov(qword_ptr[rsp + 8], rbx);
		mov(rax, returnAddress);
		jmp(rax);

		L("noModifier");
		xor(eax, eax);
		ret();
	}
};

static HookFunction hookFunction([]()
{
	// The function layout and missing-handler value are verified on b3258.
	if (!xbr::IsGameBuild<3258>())
	{
		return;
	}

	// Shared integer mod-stat lookup (b3258: 0x627088). Returning zero for
	// an absent handler matches this function's existing no-mod-kit path.
	auto location = hook::get_pattern<uint8_t>("48 89 5C 24 08 57 48 83 EC 20 48 63 DA 48 8B F9 E8 ? ? ? ? 48 85 C0 75 04 33 C0 EB 30 8D 43 DC 83 F8 05");

	static VehicleModStatStub stub;
	stub.Init(reinterpret_cast<uintptr_t>(location));
	hook::jump(location, stub.GetCode());
});
