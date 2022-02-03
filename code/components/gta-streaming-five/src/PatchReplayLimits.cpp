#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>
#include <MinHook.h>

#include <CrossBuildRuntime.h>

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

template<int IPLCount = 256>
struct CPacketIPLArray
{
	union
	{
		uint32_t icount;
		struct
		{
			uint16_t size; // +0
			uint16_t count; // +2
		};
	};
	uint32_t buffer[IPLCount]; // +4
};

/// <summary>
/// Inferred from 0x1404C41B0 (2372 retail)
/// </summary>
template<int IPLCount = 256>
struct CPacketIPLBuffer
{
	uint32_t magic; // +0
	uint32_t size; // +4
	uint32_t f_0x8; // +8
	uint32_t f_0xc; // +12
	char f_0x10[8]; // +16
	CPacketIPLArray<IPLCount> prev; // +24;
	CPacketIPLArray<IPLCount> next; // +1052
};

using CPacketIPLBufferNew = CPacketIPLBuffer<1024>;
using CPacketIPLArrayNew = CPacketIPLArray<1024>;

/// <summary>
/// Relocated IPL Buffer.
/// </summary>
static CPacketIPLBufferNew* g_IPLBuffer = nullptr;
static CPacketIPLArrayNew* g_IPLTempArray = nullptr;

/// <summary>
/// Initialize/Clear function added for g_IPLBuffer added in 1868
/// </summary>
using CPacketBufferInitialize = void*(*)(void*, int16_t, int, char);
static CPacketBufferInitialize g_InitializeBuffer = nullptr;

static CPacketIPLBuffer<>* (*g_origInitializeIPLBuffer)() = nullptr;
static void* InitializeIPLBuffer()
{
	g_origInitializeIPLBuffer(); // posterity

	void* result = g_InitializeBuffer((void*)g_IPLBuffer, 287, sizeof(CPacketIPLBufferNew), 0);
	g_IPLBuffer->prev.icount = 0;
	g_IPLBuffer->next.icount = 0;
	return result;
}

static HookFunction hookFunction([]()
{
	static const constexpr uint32_t BufferSize = sizeof(CPacketIPLBufferNew);

	// Relocate CPacketIPL buffer references; stubs from 2372
	g_IPLBuffer = (CPacketIPLBufferNew*)hook::AllocateStubMemory((size_t)BufferSize);
	RelocateRelative((void*)g_IPLBuffer, {
		{ "44 0F B7 C0 49 C1 E0 02 E8 ? ? ? ? 0F B6 CB 85 C0", -0x3C + 3 }, // 0x1401033F7 48 8D 0D 32 05 E9 01
		{ "48 8B 0B 8B 43 3C 01 01 48 8D 1D ? ? ? ? 48 8D 35", -0xC + 3 }   // 0x1401358F3 48 8D 15 36 E0 E5 01
	});

	if (xbr::IsGameBuildOrGreater<1868>())
	{
		g_IPLTempArray = (CPacketIPLArrayNew*)hook::AllocateStubMemory(sizeof(CPacketIPLArrayNew));
		RelocateRelative((void*)(&g_IPLTempArray->buffer), {
			{ "89 1D ? ? ? ? C6 05 ? ? ? ? ? 48 83 C4 20 5B", -0x10 + 3 },  // 0x1401488F3 48 8D 0D 0A AC E4 01    lea     rcx, unk_141F93504
			{ "44 0F B7 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 4C 24", 0x8 + 3 } // 0x140131FF4 48 8D 15 09 15 E6 01    lea     rdx, unk_141F93504
		});

		RelocateRelative((void*)(&g_IPLTempArray->icount), {
			{ "89 1D ? ? ? ? C6 05 ? ? ? ? ? 48 83 C4 20 5B", 2 },                     // 0x140148903 89 1D F7 AB E4 01         mov     cs:dword_141F93500, ebx
			{ "44 0F B7 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 4C 24", 4 },                 // 0x140131FEC 44 0F B7 05 0C 15 E6 01   movzx   r8d, word ptr cs:dword_141F93500
			{ "83 25 ? ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? 45 33 C0 E8", 2, 5 }, // 0x14012738B 83 25 6E C1 E6 01 00      and     cs:dword_141F93500, 0
		});
	}

	// Update offsets and fields
	hook::put<uint32_t>(hook::get_pattern("BB 20 08 00 00 41 8B D6 48 8B CE", 1), BufferSize);                       // 0x140100DF4 BB 20 08 00 00      mov     ebx, 820h;
	hook::put<uint32_t>(hook::get_pattern("B9 ? ? ? ? 48 8B 07 89 4F 3C 4C 8B 40 08", 1), BufferSize);               // 0x1400A39E9 B9 20 08 00 00      mov     ecx, 820h
	hook::put<uint32_t>(hook::get_pattern("49 03 18 48 0B C3 4C 8B C3 83 E0 0F", 0xE + 1), (BufferSize - 32) / 128); // 0x1400A3A0B B8 10 00 00 00      mov     eax, 10h
	hook::put<uint32_t>(hook::get_pattern("C7 87 38 01 00 00 20 08 00 00", 0x6), BufferSize);                        // 0x1400B009A C7 87 38 01 00 00 20 08 00 00 mov     dword ptr [rdi+138h], 820h

	{
		auto pattern = hook::pattern("81 61 04 20 08 00 FF 81 49 04 20 08 00 00").count(1).get(0);
		hook::put<uint32_t>(pattern.get<void*>(0x3), 0xFF000000 | BufferSize);                           // 0x1404C41BA 81 61 04 20 08 00 FF    and     dword ptr [rcx+4], 0FF000820h
		hook::put<uint32_t>(pattern.get<void*>(0x7 + 0x3), BufferSize);                                  // 0x1404C41C1 81 49 04 20 08 00 00    or      dword ptr [rcx+4], 820h
		hook::put<uint32_t>(pattern.get<void*>(0x32 + 0x2), offsetof(CPacketIPLBufferNew, next.size));   // 0x1404C41EC 89 81 1C 04 00 00       mov     [rcx+41Ch], eax
		hook::put<uint32_t>(pattern.get<void*>(0x49 + 0x3), offsetof(CPacketIPLBufferNew, next.buffer)); // 0x1404C4203 48 81 C1 20 04 00 00    add     rcx, 420h
	}

	if (xbr::IsGameBuildOrGreater<1868>())
	{
		auto pattern = hook::pattern("48 8D 91 20 04 00 00 8B 89 1C 04 00 00").count(1).get(0);
		hook::put<uint32_t>(pattern.get<void*>(0x0 + 0x3), offsetof(CPacketIPLBufferNew, next.buffer)); // 0x1404BF552 48 8D 91 20 04 00 00     lea     rdx, [rcx+420h]
		hook::put<uint32_t>(pattern.get<void*>(0x7 + 0x2), offsetof(CPacketIPLBufferNew, next.size));   // 0x1404BF559 8B 89 1C 04 00 00        mov     ecx, [rcx+41Ch]
	}

	// Hook initializer function
	if (xbr::IsGameBuildOrGreater<1868>())
	{
		auto pattern = hook::pattern("45 33 C9 BA ? ? ? ? 41 B8 20 08 00 00 E8").count(1).get(0);
		g_InitializeBuffer = (CPacketBufferInitialize)hook::get_call(pattern.get<void>(0xE));

		MH_Initialize();
		MH_CreateHook(pattern.get<void>(-0xB), InitializeIPLBuffer, (void**)&g_origInitializeIPLBuffer);
		MH_EnableHook(MH_ALL_HOOKS);
	}
});
