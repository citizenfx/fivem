#include "StdInc.h"

#include "CrossBuildRuntime.h"
#include "Hooking.h"
#include "NetGameEvent.h"

#include <jitasm.h>

template<int Build>
static int GetServerId(const rlGamerInfo<Build>& platformData)
{
	return (platformData.peerAddress.localAddr().ip.addr & 0xFFFF) ^ 0xFEED;
}

static int DoGetServerId(CNetGamePlayer* player)
{
	if (xbr::IsGameBuildOrGreater<2824>())
	{
		return GetServerId(*player->GetGamerInfo<2824>());
	}

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		return GetServerId(*player->GetGamerInfo<2372>());
	}

	if (xbr::IsGameBuildOrGreater<2060>())
	{
		return GetServerId(*player->GetGamerInfo<2060>());
	}

	return GetServerId(*player->GetGamerInfo<1604>());
}

static constexpr char pattern[] = "\x41\xB8\x0D\x00\x00\x00";
static auto FindPattern(const uintptr_t startAddress)
{
	auto address = reinterpret_cast<const uint8_t*>(startAddress);

	while (true)
	{
		assert(*address != 0xE9);

		if (memcmp(address, pattern, sizeof(pattern) - 1) == 0)
		{
			return reinterpret_cast<uintptr_t>(address);
		}

		++address;
	}
}

static uint32_t GetSceneIdOffset(CNetGamePlayer* player)
{
	return (DoGetServerId(player) - 1) * 64;
}

static HookFunction hookFunction([]()
{
	// StartScene - Serializer
	{
		//Write
		hook::put<uint32_t>(hook::get_pattern("41 B8 ? ? ? ? 48 8B C8 C6 44 24 ? ? E8 ? ? ? ? 48 8B 7C 24", 2), 32);

		//Read
		hook::put<uint32_t>(hook::get_pattern("41 B8 ? ? ? ? 48 8B C8 48 8B D6", 2), 32);
	}

	// Request-/StopScene - Serializer
	{
		const auto cCRequestNetworkSyncedSceneEvent_vtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("48 8D 05 ? ? ? ? 89 5E ? 48 8B 5C 24 ? 48 89 06 8A", 3));

		// Write
		uintptr_t target = cCRequestNetworkSyncedSceneEvent_vtable[5];

		hook::put<uint32_t>(FindPattern(target) + 2, 32);

		// Read
		target = cCRequestNetworkSyncedSceneEvent_vtable[6];

		hook::put<uint32_t>(FindPattern(target) + 2, 32);
	}

	// UpdateScene - Serializer
	{
		const auto cCUpdateNetworkSyncedSceneEvent_vtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? F3 0F 11 77 ? 0F 28 74 24", 3));

		// Write
		uintptr_t target = cCUpdateNetworkSyncedSceneEvent_vtable[5];
		target = target + 5 + *reinterpret_cast<int32_t*>(target + 1);

		hook::put<uint32_t>(FindPattern(target) + 2, 32);

		// Read
		target = cCUpdateNetworkSyncedSceneEvent_vtable[6];
		target = target + 5 + *reinterpret_cast<int32_t*>(target + 1);

		hook::put<uint32_t>(FindPattern(target) + 2, 32);
	}

	// CClonedSynchronizedSceneInfo - Serializer
	{
		const auto cClonedSynchronizedSceneInfo_vtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? E9 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 C9 4D 8B C6 BA ? ? ? ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B C8 E8 ? ? ? ? E9 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 C9 4D 8B C6 BA", 3));

		int vtableIdx = 24;

		if (xbr::IsGameBuildOrGreater<2802>())
		{
			vtableIdx = 30;
		}

		hook::put<uint8_t>(cClonedSynchronizedSceneInfo_vtable[vtableIdx] + 0x1F + 3, 32);
	}

	// CNetworkSynchronizedScenes - Scene Allocator
	{
		static struct : jitasm::Frontend
		{
			intptr_t retAddress;

			void Init(const intptr_t ret)
			{
				this->retAddress = ret;
			}

			void InternalMain() override
			{
				mov(rcx, rax);
				mov(rax, reinterpret_cast<uintptr_t>(&GetSceneIdOffset));
				call(rax);
				mov(ebp, eax);

				mov(rax, retAddress);
				jmp(rax);
			}
		} patchStub;

		if (xbr::IsGameBuildOrGreater<xbr::Build::Summer_2025>())
		{
			char* location = hook::get_pattern<char>("0F B6 A8 ? ? ? ? C1 E5");
			const auto ret = reinterpret_cast<intptr_t>(location) + 10;

			patchStub.Init(ret);

			hook::nop(location, 10);
			hook::jump_rcx(location, patchStub.GetCode());
		}
		else
		{
			char* location = hook::get_pattern<char>("0F B6 68 ? C1 E5");

			const auto ret = reinterpret_cast<intptr_t>(location) + 7;

			patchStub.Init(ret);

			hook::nop(location, 7);
			hook::jump_rcx(location, patchStub.GetCode());
		}
	}
});
