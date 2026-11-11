#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <MinHook.h>
#include <NetLibrary.h>

//
// In RDR3, Ped groups are limited to 68.
// 32 for players and the rest for peds/posses. For the sake of compatability we want to increase this to 164 (128 players + 36 for peds/posses)
//

struct CPedGroup
{
	char pad[4832];
};

static const uint8_t kMaxPedGroups = 128 + 36;
static CPedGroup* g_pedGroups;

//TODO: These really need to be moved to some shared file.
struct PatternPair
{
	std::string_view pattern;
	int offset;
	int operand_remaining = 4;
};

static void RelocateRelative(void* base, std::initializer_list<PatternPair> list)
{
	void* oldAddress = nullptr;
	for (auto& entry : list)
	{
		auto location = hook::get_pattern<int32_t>(entry.pattern, entry.offset);

		if (!oldAddress)
		{
			oldAddress = hook::get_address<void*>(location, 0, entry.operand_remaining);
		}

		auto curTarget = hook::get_address<void*>(location, 0, entry.operand_remaining);
		assert(curTarget == oldAddress);

		hook::put<int32_t>(location, (intptr_t)base - (intptr_t)location - entry.operand_remaining);
	}
}

struct PatternAbsolutePair
{
	std::string_view pattern;
	int offset;
	uintptr_t address;
	int instrLen = 7;
};

static void RelocateRelativeLocation(std::initializer_list<PatternAbsolutePair> list)
{
	uintptr_t oldAddress = 0;

	for (auto& entry : list)
	{
	    uint8_t* instructions = reinterpret_cast<uint8_t*>(hook::get_pattern(entry.pattern));
		uintptr_t instrNext = reinterpret_cast<uintptr_t>(instructions) + entry.instrLen;
		int32_t newDisp = (int32_t)((int64_t)entry.address- (int64_t)instrNext);
		hook::put<int32_t>(instructions + entry.offset, newDisp);
	}
}

template<int instrLen = 7, int instrOffset = 3>
static void PatchRelativeLocation(uintptr_t address, uintptr_t newLocation)
{
    uint8_t* instructions = reinterpret_cast<uint8_t*>(address);
	uintptr_t instrNext = address + instrLen;
	int32_t newDisp = (int32_t)((int64_t)newLocation - (int64_t)instrNext);
	hook::put<int32_t>(instructions + instrOffset, newDisp);
}

struct PatternPatchPair
{
	std::string_view pattern;
	int offset;
	int intendedValue;
	int newValue;
};

template<class T>
static void PatchValue(std::initializer_list<PatternPatchPair> list)
{
	for (auto& entry : list)
	{
		auto location = hook::pattern(entry.pattern).count(1).get(0).get<T>(entry.offset);
		auto origVal = *location;
		assert(origVal == entry.intendedValue);
		hook::put<T>(location, (T)entry.newValue);
	}
}

static HookFunction hookFunction([]()
{
	g_pedGroups = (CPedGroup*)hook::AllocateStubMemory(sizeof(CPedGroup) * kMaxPedGroups + 4);
	uintptr_t g_unkEndPtr = reinterpret_cast<uintptr_t>(g_pedGroups + 68);

	// cmp ebx, 0x44 (signed 8-bit comaparsion) -> cmp ebx, kMaxPedGroups (32-bit)
	// this function also has a 32 sized player list access, but this is patched inside of netPlayerArrayPatch.
	{
		auto location = hook::get_pattern("83 FB ? 72 ? 80 3D");
		static struct : jitasm::Frontend
		{
			uintptr_t retnBreak;
			uintptr_t retnContinue;

			void Init(uintptr_t continueLoop, uintptr_t breakLoop)
			{
				this->retnContinue = continueLoop;
				this->retnBreak = breakLoop;
			}

			virtual void InternalMain() override
			{
				cmp(ebx, (uint32_t)kMaxPedGroups);
				jb("LoopContinue");

				mov(rax, retnBreak);
				jmp(rax);

				L("LoopContinue");
				mov(rax, retnContinue);
				jmp(rax);
			}
		} patchStub;

		const uintptr_t retnBreak = (uintptr_t)location + 5;
		const uintptr_t retnContinue = (uintptr_t)hook::get_pattern("8B CB 88 9F");

		hook::nop(location, 5);
		patchStub.Init(retnContinue, retnBreak);
		hook::jump(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("83 FB ? 72 ? 0F 28 C6 48 8B 5C 24 ? 48 8B 74 24");

		static struct : jitasm::Frontend
		{
			uintptr_t retnBreak;
			uintptr_t retnContinue;

			void Init(uintptr_t continueLoop, uintptr_t breakLoop)
			{
				this->retnContinue = continueLoop;
				this->retnBreak = breakLoop;
			}

			virtual void InternalMain() override
			{
				cmp(ebx, (uint32_t)kMaxPedGroups);
				jb("LoopContinue");

				mov(rax, retnBreak);
				jmp(rax);

				L("LoopContinue");
				mov(rax, retnContinue);
				jmp(rax);
			}
		} patchStub;

		const uintptr_t retnBreak = (uintptr_t)location + 5;
		const uintptr_t retnContinue = (uintptr_t)hook::get_pattern("F6 07 ? 74 ? 8B C3 48 69 F0");

		hook::nop(location, 5);
		patchStub.Init(retnContinue, retnBreak);
		hook::jump(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("83 F8 ? 73 ? 44 8B 0D");

		static struct : jitasm::Frontend
		{
			uintptr_t retnBreak;
			uintptr_t retnContinue;

			void Init(uintptr_t continueLoop, uintptr_t breakLoop)
			{
				this->retnContinue = continueLoop;
				this->retnBreak = breakLoop;
			}

			virtual void InternalMain() override
			{
				cmp(ebx, (uint32_t)kMaxPedGroups);
				jnb("LoopBreak");

				mov(rax, retnContinue);
				jmp(rax);

				L("LoopBreak");
				mov(rax, retnBreak);
				jmp(rax);
			}
		} patchStub;

		const uintptr_t retnContinue = (uintptr_t)location + 5;
		const uintptr_t retnBreak = (uintptr_t)hook::get_pattern("44 88 35 ? ? ? ? EB ? 48 8B 02");

		hook::nop(location, 5);
		patchStub.Init(retnContinue, retnBreak);
		hook::jump(location, patchStub.GetCode());
	}

	static struct SharedPatch : jitasm::Frontend
	{
		uintptr_t retn;
		uintptr_t offset;

		void Init(uintptr_t retn, uintptr_t offset)
		{
			this->retn = retn;
			this->offset = offset;
		}

		virtual void InternalMain() override
		{
			cmp(dword_ptr[rsp + offset], kMaxPedGroups);
			mov(rax, retn);
			jmp(rax);
		}
	};

	{
		auto location = hook::get_pattern("83 7C 24 ? ? 73 ? 8B 44 24 ? 48 69 C8 ? ? ? ? 48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 74 ? 48 8D 1C 01 48 8B CB");

		SharedPatch patchStub;

		hook::nop(location, 5);
		patchStub.Init((uintptr_t)location + 5, 0x40);
		hook::jump(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("83 7C 24 ? ? 73 ? 8B 44 24 ? 48 69 C8 ? ? ? ? 48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 74 ? 48 8D 1C 01 EB");
		SharedPatch patchStub;
		hook::nop(location, 5);
		patchStub.Init((uintptr_t)location + 5, 0x40);
		hook::jump(location, patchStub.GetCode());
	}

	{
		auto location = hook::get_pattern("83 79 ? ? 48 8B F1 0F 83 ? ? ? ? E8");
		static struct : jitasm::Frontend
		{
			uintptr_t retn;

			void Init(uintptr_t retn)
			{
				this->retn = retn;
			}

			virtual void InternalMain() override
			{
				cmp(dword_ptr[rsp + 0x20], kMaxPedGroups);
				mov(rsi, rcx);

				mov(rax, retn);
				jmp(rax);
			}
		} patchStub;

		hook::nop(location, 7);
		patchStub.Init((uintptr_t)location + 7);
		hook::jump(location, patchStub.GetCode());
	} 

	// 0x18
	{
		uintptr_t newTarget = reinterpret_cast<uintptr_t>(g_pedGroups) + 0x18;
		PatchRelativeLocation((uintptr_t)hook::get_pattern("48 8D 3D ? ? ? ? 8D 73 ? 8B CB"), newTarget);
	}

	// 0x28
	{
		uintptr_t newTarget = reinterpret_cast<uintptr_t>(g_pedGroups) + 0x28;
		RelocateRelativeLocation({
			{ "48 8D 05 ? ? ? ? 48 03 C8 E8 ? ? ? ? 83 F8", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 83 E2 ? 48 03 C8", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 03 C8 74 ? 48 8B D3 E8 ? ? ? ? 84 C0", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 03 C8 E8 ? ? ? ? 41 8B 86", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 03 C8 E8 ? ? ? ? 48 83 C4 ? 5B C3 CC", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 03 C8 E8 ? ? ? ? 84 C0 74 ? 32 C0", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 03 F8 33 DB", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 03 C8 E8 ? ? ? ? F6 D8", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 8B D6 48 03 C8 E8 ? ? ? ? 84 C0", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 8B D7 48 03 C8 E8 ? ? ? ? 84 C0", 3, newTarget },
			{ "48 8D 15 ? ? ? ? 48 69 C9 ? ? ? ? 45 33 C0", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 48 03 C8 E8 ? ? ? ? 65 48 8B 04 25 ? ? ? ? 48 8B 0C F8", 3, newTarget }
		});
	}

	// 0x1034
	{
		uintptr_t newTarget = reinterpret_cast<uintptr_t>(g_pedGroups) + 0x1034;
		RelocateRelativeLocation({
			{ "48 8D 05 ? ? ? ? 44 01 24 01", 3, newTarget },
			{ "48 8D 05 ? ? ? ? 44 01 3C 01", 3, newTarget },
			{ "48 8D 05 ? ? ? ? FF 04 01", 3, newTarget }
		});
	}

	// 0x1260
	{
		uintptr_t newTarget = reinterpret_cast<uintptr_t>(g_pedGroups) + 0x1260;
		PatchRelativeLocation((uintptr_t)hook::get_pattern("48 8D 1D ? ? ? ? BE ? ? ? ? 48 8B 3B"), newTarget);
	}

	// 0x12A0
	{
		uintptr_t newTarget = reinterpret_cast<uintptr_t>(g_pedGroups) + 0x12A0;

		RelocateRelativeLocation({ 
			{ "4C 8D 0D ? ? ? ? BD", 3, newTarget},
			{ "4C 8D 05 ? ? ? ? 49 03 C8 F6 01", 3, newTarget},
			{ "48 8D 3D ? ? ? ? F3 0F 10 35 ? ? ? ? 33 DB", 3, newTarget}
		});	 
	}

	// 0x12B8
	{
		uintptr_t newTarget = reinterpret_cast<uintptr_t>(g_pedGroups) + 0x12B8;
		PatchRelativeLocation((uintptr_t)hook::get_pattern("48 8D 05 ? ? ? ? 48 03 C8 C6 01"), newTarget);
	}

	//TODO: Improve pattern
	auto location = hook::get_pattern("48 8D 3D ? ? ? ? 48 81 EF ? ? ? ? 48 8B CF E8 ? ? ? ? 48 83 EB ? 75 ? 48 8B 5C 24 ? 48 83 C4 ? 5F C3 CC CC 48 8D 0D ? ? ? ? E9 ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ? C2", 3);
	PatchRelativeLocation((uintptr_t)location, g_unkEndPtr);
	PatchRelativeLocation((uintptr_t)hook::get_pattern("C7 44 24 44 B9 1B 1C 78 49 89 43 F0 48 8D 0D BA 07 81 04"), g_unkEndPtr);

	RelocateRelative(g_pedGroups, { 
		// TODO: improve pattern
		{ "48 8D 1D ? ? ? ? BF ? ? ? ? 48 8B CB E8 ? ? ? ? 48 81 C3 ? ? ? ? 48 83 EF ? 75 ? 48 8D 0D ? ? ? ? 48 8B 5C 24 ? 48 83 C4 ? 5F E9 ? ? ? ? CC CC CC 48 83 EC ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 48 83 C4 ? E9 ? ? ? ? 48 83 EC ? 48 8D 0D", 3 },
		
		{ "48 8D 1D ? ? ? ? BF ? ? ? ? 8B D5", 3 },
		{ "4C 8D 35 ? ? ? ? 48 69 F0 ? ? ? ? 33 D2", 3 },
		{ "48 8D 05 ? ? ? ? 33 D2 48 03 D8", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 E8 ? ? ? ? C6 83", 3 },
		{ "48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 48 83 C4", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 F0 48 8B CE", 3 },
		{ "48 8D 05 ? ? ? ? 48 69 D1 ? ? ? ? 48 03 D0", 3},
		{ "48 8D 1D ? ? ? ? BF ? ? ? ? F6 83", 3 },
		{ "48 8D 1D ? ? ? ? F3 0F 10 35 ? ? ? ? BF", 3 },
		{ "48 8D 05 ? ? ? ? 44 84 84 01", 3 },
		{ "48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 74 ? 48 03 C8", 3 },
		{ "48 8D 1D ? ? ? ? BF ? ? ? ? 33 D2", 3 },
		{ "4C 8D 25 ? ? ? ? BE ? ? ? ? 39 9F", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 C8 EB ? 33 C9 48 85 C9 74 ? 8A 81", 3},
		{ "48 8D 05 ? ? ? ? 48 03 E8 EB", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 F0 EB", 3 },
		{ "48 8D 0D ? ? ? ? 48 03 F9", 3 },
		{ "4C 8D 15 ? ? ? ? 48 83 E3", 3},
		{ "48 8D 0D ? ? ? ? 48 03 C1 C3 FF C8", 3},
		{ "4C 8D 0D ? ? ? ? B9 ? ? ? ? 45 85 C0", 3 },
		{ "48 8D 2D ? ? ? ? BF ? ? ? ? 85 C0", 3 },
		{ "48 8D 2D ? ? ? ? 80 3D ? ? ? ? ? 74", 3 },
		{ "48 8D 05 ? ? ? ? 48 69 C9 ? ? ? ? 48 03 C8 48 8B 41", 3 },
		{ "48 8D 0D ? ? ? ? 48 03 D9", 3 },
		{ "48 8D 0D ? ? ? ? 48 03 C1 EB", 3 },
		{ "48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 74 ? FF 8C 01 ? ? ? ? 4D 8B 50", 3},
		{ "48 8D 05 ? ? ? ? 41 B0 ? 48 69 DF", 3 },
		{ "48 8D 05 ? ? ? ? 48 69 FE", 3 },
		{ "48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 74 ? FF 8C 01 ? ? ? ? 48 8B 5C 24 ? 48 8B 74 24 ? 48 83 C4 ? 5F C3 90", 3},
		{ "48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 74 ? FF 8C 01 ? ? ? ? 48 8B 5C 24 ? 48 8B 74 24 ? 48 83 C4 ? 5F C3 CC", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 D8 F6 83 ? ? ? ? ? 0F 84", 3 },
		{ "48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 0F 84", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 F0 48 89 75", 3 },
		{ "48 8D 05 ? ? ? ? 4C 03 E8 41 38 B5", 3 },
		{ "48 8D 05 ? ? ? ? 33 D2 48 03 C8", 3 },
		{ "48 8D 05 ? ? ? ? 48 69 DB ? ? ? ? 48 8D 54 24", 3 },
		{ "48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 74 ? 48 8D 1C 01 48 8B CB", 3 },
		{ "48 8D 05 ? ? ? ? F6 84 01 ? ? ? ? ? 74 ? 48 8D 1C 01 EB", 3},
		{ "48 8D 0D ? ? ? ? 48 69 D2", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 F8 74", 3},
		{ "48 8D 05 ? ? ? ? 48 03 D8 74", 3},
		{ "48 8D 05 ? ? ? ? 48 69 D9 ? ? ? ? 48 03 D8", 3 },
		{ "48 8D 0D ? ? ? ? F6 84 08", 3 },
		{ "48 8D 05 ? ? ? ? 48 69 F9 ? ? ? ? 48 03 F8", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 D8 EB ? 33 DB", 3 },
		{ "48 8D 05 ? ? ? ? 48 03 F0 0F 84 ? ? ? ? 48 8D 4E", 3 }
	});

	// Patch array initalization
	PatchValue<uint32_t>({
		//TODO: Improve pattern
		{ "BF ? ? ? ? 48 8B CB E8 ? ? ? ? 48 81 C3 ? ? ? ? 48 83 EF ? 75 ? 48 8D 0D ? ? ? ? 48 8B 5C 24 ? 48 83 C4 ? 5F E9 ? ? ? ? CC CC CC 48 83 EC ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 48 83 C4 ? E9 ? ? ? ? 48 83 EC ? 48 8D 0D", 1, 0x44, kMaxPedGroups},
		{ "BF ? ? ? ? 8B D5 48 8B CB E8 ? ? ? ? 48 81 C3", 1, 0x44, kMaxPedGroups },
		{ "BF ? ? ? ? F6 83 ? ? ? ? ? 74 ? 48 8B CB", 1, 0x44, kMaxPedGroups },
		{ "BF ? ? ? ? F6 83 ? ? ? ? ? 74 ? 0F 28 CE", 1, 0x44, kMaxPedGroups },
		{ "BF ? ? ? ? 33 D2 48 8B CB E8 ? ? ? ? 48 81 C3", 1, 0x44, kMaxPedGroups } // _shutdownGroups
	});

	hook::put<uint8_t>(hook::get_pattern("8D 51 ? 8B C2 7E", 2), kMaxPedGroups);
});
