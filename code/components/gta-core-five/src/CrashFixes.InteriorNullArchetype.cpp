/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "CrossBuildRuntime.h"
#include "Hooking.h"
#include "Hooking.Patterns.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

//
// On game build 3889+, interior/spatial traversal routines frequently dereference the
// descriptor object resolved from [CInteriorInst + 0xF0] without null-checking the result.
// When an interior instance has a null +0xF0 pointer (or its resolver returns nullptr),
// several functions crash on +0xC8 / +0xD8 / +0xE8 / +0x11A accesses:
//   - sub_140638724  (e.g. blue-iowa-salami, angel-virginia-early, magazine-one-shade)
//   - sub_14063CE..  (e.g. venus-robin-missouri)
//   - Switch012 blocks in interior/portal lookups (e.g. august-lima-fifteen, failed-muppet-xray)
//
// This fix:
//   1. Guards the known control-flow sites in sub_140638724 and its callee so they take their
//      existing null/fallback branches cleanly.
//   2. Patches remaining unguarded [reg+0xF0] resolver callers and 0/1/2 LOD-portal switches to
//      fall back to a safe static dummy descriptor (0 rooms, 0 portals, valid refcounted vtable).
//

namespace
{
struct PatternAMatch
{
	uintptr_t addr;
	size_t prefixLen; // 24 for `EB 03 48 8B C3` (mov rax, rbx), 23 for `EB 02 33 C0` (xor eax, eax)
};

struct DummyEnvironment
{
	uintptr_t dummyF0Addr;
	uintptr_t dummyDataAddr;
};

static alignas(16) uint8_t g_dummyDataMem[16384]{};

static DummyEnvironment BuildDummyEnvironment(uint8_t* execPage)
{
	std::memset(g_dummyDataMem, 0, sizeof(g_dummyDataMem));

	// Executable dummy virtual method: xor eax, eax; ret
	execPage[0] = 0x33;
	execPage[1] = 0xC0;
	execPage[2] = 0xC3;
	execPage[3] = 0x90;
	const uintptr_t retZeroFn = reinterpret_cast<uintptr_t>(execPage);

	// Layout inside g_dummyDataMem:
	//   +0x0000: DummyVtable[128] (1024 B)
	//   +0x0400: DummyVtableObj   (512 B) -> [0] = &DummyVtable, +0x32 = 0x4000 (pgBase refcount)
	//   +0x0800: DummyRoom        (512 B) -> [0] = &DummyVtable, +0x32 = 0x4000
	//   +0x0C00: DummyPtrTable[64]
	//   +0x1000: DummyModelInfo   (512 B) -> [0] = &DummyVtable, +0x32 = 0x4000
	//   +0x1400: DummyEntity      (512 B) -> [0] = &DummyVtable, +0x20 = &DummyModelInfo, +0x32 = 0x4000
	//   +0x1800: DummyData        (512 B) -> [0] = &DummyVtable, +0x32 = 0x4000, [+0xC8/+0xD0/+0xD8/+0xE8], +0x11A=0, +0x11E=0
	//   +0x1C00: DummyF0          (256 B) -> [+0x10/+0x18/+0x20] = &DummyData
	auto* vtable = reinterpret_cast<uint64_t*>(g_dummyDataMem + 0x0000);
	for (int i = 0; i < 128; ++i)
	{
		vtable[i] = retZeroFn;
	}
	const uintptr_t vtableAddr = reinterpret_cast<uintptr_t>(vtable);

	auto initRefObject = [vtableAddr](uint8_t* obj)
	{
		*reinterpret_cast<uint64_t*>(obj) = vtableAddr;
		*reinterpret_cast<uint16_t*>(obj + 0x32) = 0x4000;
	};

	uint8_t* vtableObj = g_dummyDataMem + 0x0400;
	initRefObject(vtableObj);

	uint8_t* roomObj = g_dummyDataMem + 0x0800;
	initRefObject(roomObj);

	auto* ptrTable = reinterpret_cast<uint64_t*>(g_dummyDataMem + 0x0C00);
	for (int i = 0; i < 64; ++i)
	{
		ptrTable[i] = reinterpret_cast<uintptr_t>(roomObj);
	}

	uint8_t* modelInfo = g_dummyDataMem + 0x1000;
	initRefObject(modelInfo);

	uint8_t* entityObj = g_dummyDataMem + 0x1400;
	initRefObject(entityObj);
	*reinterpret_cast<uint64_t*>(entityObj + 0x20) = reinterpret_cast<uintptr_t>(modelInfo);

	uint8_t* dummyData = g_dummyDataMem + 0x1800;
	initRefObject(dummyData);
	*reinterpret_cast<uint64_t*>(dummyData + 0xC8) = reinterpret_cast<uintptr_t>(ptrTable);
	*reinterpret_cast<uint64_t*>(dummyData + 0xD0) = reinterpret_cast<uintptr_t>(ptrTable);
	*reinterpret_cast<uint64_t*>(dummyData + 0xD8) = reinterpret_cast<uintptr_t>(entityObj);
	*reinterpret_cast<uint64_t*>(dummyData + 0xE8) = reinterpret_cast<uintptr_t>(vtableObj);

	uint8_t* dummyF0 = g_dummyDataMem + 0x1C00;
	*reinterpret_cast<uint64_t*>(dummyF0 + 0x10) = reinterpret_cast<uintptr_t>(dummyData);
	*reinterpret_cast<uint64_t*>(dummyF0 + 0x18) = reinterpret_cast<uintptr_t>(dummyData);
	*reinterpret_cast<uint64_t*>(dummyF0 + 0x20) = reinterpret_cast<uintptr_t>(dummyData);

	return DummyEnvironment{
		reinterpret_cast<uintptr_t>(dummyF0),
		reinterpret_cast<uintptr_t>(dummyData)
	};
}

static bool EmitRelJump(uintptr_t hookAddr, void* stubAddr, size_t nopLen)
{
	if (nopLen < 5)
	{
		return false;
	}

	const intptr_t diff = reinterpret_cast<intptr_t>(stubAddr) - static_cast<intptr_t>(hookAddr + 5);
	if (diff < std::numeric_limits<int32_t>::min() || diff > std::numeric_limits<int32_t>::max())
	{
		return false;
	}

	hook::nop(hookAddr, nopLen);
	hook::putVP<uint8_t>(hookAddr, 0xE9);
	hook::putVP<int32_t>(hookAddr + 1, static_cast<int32_t>(diff));
	return true;
}

static bool PatchSite1(uintptr_t hookAddr, uint8_t* stubPage, size_t& offset)
{
	const auto* b = reinterpret_cast<const uint8_t*>(hookAddr);
	static const uint8_t kSig[] = { 0x48, 0x8B, 0xB0, 0xD8, 0x00, 0x00, 0x00, 0x48, 0x85, 0xF6, 0x0F, 0x84 };
	if (std::memcmp(b, kSig, sizeof(kSig)) != 0)
	{
		return false;
	}

	const uintptr_t resumeAddr = hookAddr + 0x10;
	const int32_t relExit = *reinterpret_cast<const int32_t*>(hookAddr + 0x0C);
	const uintptr_t exitAddr = resumeAddr + static_cast<intptr_t>(relExit);

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC0;                                 // test rax, rax
	s[3] = 0x74; s[4] = 0x1A;                                              // jz +0x1A -> 0x1F
	s[5] = 0x48; s[6] = 0x8B; s[7] = 0xB0;                                 // mov rsi, [rax+0xD8]
	s[8] = 0xD8; s[9] = 0x00; s[10] = 0x00; s[11] = 0x00;
	s[12] = 0x48; s[13] = 0x85; s[14] = 0xF6;                              // test rsi, rsi
	s[15] = 0x74; s[16] = 0x10;                                            // jz +0x10 -> 0x21
	s[17] = 0xFF; s[18] = 0x25; s[19] = 0; s[20] = 0; s[21] = 0; s[22] = 0;
	*reinterpret_cast<uint64_t*>(s + 23) = resumeAddr;
	s[31] = 0x33; s[32] = 0xF6;                                            // xor esi, esi
	s[33] = 0xFF; s[34] = 0x25; s[35] = 0; s[36] = 0; s[37] = 0; s[38] = 0;
	*reinterpret_cast<uint64_t*>(s + 39) = exitAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, 16);
}

static bool PatchSite2(uintptr_t hookAddr, uint8_t* stubPage, size_t& offset)
{
	const auto* b = reinterpret_cast<const uint8_t*>(hookAddr);
	static const uint8_t kSig[] = { 0x44, 0x38, 0xA8, 0x1A, 0x01, 0x00, 0x00, 0x75 };
	if (std::memcmp(b, kSig, sizeof(kSig)) != 0)
	{
		return false;
	}

	const uintptr_t resumeAddr = hookAddr + 9;
	const int8_t relExit = *reinterpret_cast<const int8_t*>(hookAddr + 8);
	const uintptr_t exitAddr = resumeAddr + static_cast<intptr_t>(relExit);

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC0;                                 // test rax, rax
	s[3] = 0x74; s[4] = 0x17;                                              // jz +0x17 -> 0x1C
	s[5] = 0x44; s[6] = 0x38; s[7] = 0xA8;                                 // cmp [rax+0x11A], r13b
	s[8] = 0x1A; s[9] = 0x01; s[10] = 0x00; s[11] = 0x00;
	s[12] = 0x75; s[13] = 0x0E;                                            // jnz +0x0E -> 0x1C
	s[14] = 0xFF; s[15] = 0x25; s[16] = 0; s[17] = 0; s[18] = 0; s[19] = 0;
	*reinterpret_cast<uint64_t*>(s + 20) = resumeAddr;
	s[28] = 0xFF; s[29] = 0x25; s[30] = 0; s[31] = 0; s[32] = 0; s[33] = 0;
	*reinterpret_cast<uint64_t*>(s + 34) = exitAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, 9);
}

static bool PatchSite3(uintptr_t hookAddr, uint8_t* stubPage, size_t& offset)
{
	const auto* b = reinterpret_cast<const uint8_t*>(hookAddr);
	static const uint8_t kSig[] = {
		0x48, 0x8B, 0x80, 0xC8, 0x00, 0x00, 0x00,
		0x48, 0x8B, 0x08,
		0x44, 0x84, 0x69, 0x53,
		0x74
	};
	if (std::memcmp(b, kSig, sizeof(kSig)) != 0)
	{
		return false;
	}

	const uintptr_t resumeAddr = hookAddr + 16;
	const int8_t relExit = *reinterpret_cast<const int8_t*>(hookAddr + 15);
	const uintptr_t exitAddr = resumeAddr + static_cast<intptr_t>(relExit);

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC0;                                 // test rax, rax
	s[3] = 0x74; s[4] = 0x28;                                              // jz -> 0x2D
	s[5] = 0x48; s[6] = 0x8B; s[7] = 0x80;                                 // mov rax, [rax+0xC8]
	s[8] = 0xC8; s[9] = 0x00; s[10] = 0x00; s[11] = 0x00;
	s[12] = 0x48; s[13] = 0x85; s[14] = 0xC0;                              // test rax, rax
	s[15] = 0x74; s[16] = 0x1C;                                            // jz -> 0x2D
	s[17] = 0x48; s[18] = 0x8B; s[19] = 0x08;                              // mov rcx, [rax]
	s[20] = 0x48; s[21] = 0x85; s[22] = 0xC9;                              // test rcx, rcx
	s[23] = 0x74; s[24] = 0x14;                                            // jz -> 0x2D
	s[25] = 0x44; s[26] = 0x84; s[27] = 0x69; s[28] = 0x53;                // test [rcx+0x53], r13b
	s[29] = 0x74; s[30] = 0x0E;                                            // jz -> 0x2D
	s[31] = 0xFF; s[32] = 0x25; s[33] = 0; s[34] = 0; s[35] = 0; s[36] = 0;
	*reinterpret_cast<uint64_t*>(s + 37) = resumeAddr;
	s[45] = 0xFF; s[46] = 0x25; s[47] = 0; s[48] = 0; s[49] = 0; s[50] = 0;
	*reinterpret_cast<uint64_t*>(s + 51) = exitAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, 16);
}

static bool PatchSite4(uintptr_t hookAddr, uint8_t* stubPage, size_t& offset)
{
	const auto* b = reinterpret_cast<const uint8_t*>(hookAddr);
	static const uint8_t kSig[] = { 0x0F, 0xB6, 0x80, 0x1A, 0x01, 0x00, 0x00, 0x44, 0x3B, 0xF8, 0x7D };
	if (std::memcmp(b, kSig, sizeof(kSig)) != 0)
	{
		return false;
	}

	const uintptr_t resumeAddr = hookAddr + 12;
	const int8_t relExit = *reinterpret_cast<const int8_t*>(hookAddr + 11);
	const uintptr_t exitAddr = resumeAddr + static_cast<intptr_t>(relExit);

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC0;                                 // test rax, rax
	s[3] = 0x74; s[4] = 0x1A;                                              // jz -> 0x1F
	s[5] = 0x0F; s[6] = 0xB6; s[7] = 0x80;                                 // movzx eax, byte ptr [rax+0x11A]
	s[8] = 0x1A; s[9] = 0x01; s[10] = 0x00; s[11] = 0x00;
	s[12] = 0x44; s[13] = 0x3B; s[14] = 0xF8;                              // cmp r15d, eax
	s[15] = 0x7D; s[16] = 0x0E;                                            // jge -> 0x1F
	s[17] = 0xFF; s[18] = 0x25; s[19] = 0; s[20] = 0; s[21] = 0; s[22] = 0;
	*reinterpret_cast<uint64_t*>(s + 23) = resumeAddr;
	s[31] = 0xFF; s[32] = 0x25; s[33] = 0; s[34] = 0; s[35] = 0; s[36] = 0;
	*reinterpret_cast<uint64_t*>(s + 37) = exitAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, 12);
}

static bool PatchSite5(uintptr_t hookAddr, uint8_t* stubPage, size_t& offset)
{
	const auto* b = reinterpret_cast<const uint8_t*>(hookAddr);
	static const uint8_t kSig[] = {
		0x48, 0x8B, 0x80, 0xC8, 0x00, 0x00, 0x00,
		0x4A, 0x8B, 0x0C, 0x30,
		0x0F, 0x2F, 0x71, 0x44,
		0x73
	};
	if (std::memcmp(b, kSig, sizeof(kSig)) != 0)
	{
		return false;
	}

	const uintptr_t resumeAddr = hookAddr + 17;
	const int8_t relExit = *reinterpret_cast<const int8_t*>(hookAddr + 16);
	const uintptr_t exitAddr = resumeAddr + static_cast<intptr_t>(relExit);

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC0;                                 // test rax, rax
	s[3] = 0x74; s[4] = 0x29;                                              // jz -> 0x2E
	s[5] = 0x48; s[6] = 0x8B; s[7] = 0x80;                                 // mov rax, [rax+0xC8]
	s[8] = 0xC8; s[9] = 0x00; s[10] = 0x00; s[11] = 0x00;
	s[12] = 0x48; s[13] = 0x85; s[14] = 0xC0;                              // test rax, rax
	s[15] = 0x74; s[16] = 0x1D;                                            // jz -> 0x2E
	s[17] = 0x4A; s[18] = 0x8B; s[19] = 0x0C; s[20] = 0x30;                // mov rcx, [rax+r14]
	s[21] = 0x48; s[22] = 0x85; s[23] = 0xC9;                              // test rcx, rcx
	s[24] = 0x74; s[25] = 0x14;                                            // jz -> 0x2E
	s[26] = 0x0F; s[27] = 0x2F; s[28] = 0x71; s[29] = 0x44;                // comiss xmm6, [rcx+0x44]
	s[30] = 0x73; s[31] = 0x0E;                                            // jae -> 0x2E
	s[32] = 0xFF; s[33] = 0x25; s[34] = 0; s[35] = 0; s[36] = 0; s[37] = 0;
	*reinterpret_cast<uint64_t*>(s + 38) = resumeAddr;
	s[46] = 0xFF; s[47] = 0x25; s[48] = 0; s[49] = 0; s[50] = 0; s[51] = 0;
	*reinterpret_cast<uint64_t*>(s + 52) = exitAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, 17);
}

static bool PatchSite6(uintptr_t hookAddr, uint8_t* stubPage, size_t& offset)
{
	const auto* b = reinterpret_cast<const uint8_t*>(hookAddr);
	static const uint8_t kSig[] = {
		0x48, 0x8B, 0x80, 0xC8, 0x00, 0x00, 0x00,
		0x4A, 0x8B, 0x0C, 0x30,
		0xF3, 0x0F, 0x10, 0x71, 0x44
	};
	if (std::memcmp(b, kSig, sizeof(kSig)) != 0)
	{
		return false;
	}

	const uintptr_t resumeAddr = hookAddr + 16;

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC0;                                 // test rax, rax
	s[3] = 0x74; s[4] = 0x1A;                                              // jz -> 0x1F
	s[5] = 0x48; s[6] = 0x8B; s[7] = 0x80;                                 // mov rax, [rax+0xC8]
	s[8] = 0xC8; s[9] = 0x00; s[10] = 0x00; s[11] = 0x00;
	s[12] = 0x48; s[13] = 0x85; s[14] = 0xC0;                              // test rax, rax
	s[15] = 0x74; s[16] = 0x0E;                                            // jz -> 0x1F
	s[17] = 0x4A; s[18] = 0x8B; s[19] = 0x0C; s[20] = 0x30;                // mov rcx, [rax+r14]
	s[21] = 0x48; s[22] = 0x85; s[23] = 0xC9;                              // test rcx, rcx
	s[24] = 0x74; s[25] = 0x05;                                            // jz -> 0x1F
	s[26] = 0xF3; s[27] = 0x0F; s[28] = 0x10; s[29] = 0x71; s[30] = 0x44;  // movss xmm6, [rcx+0x44]
	s[31] = 0xFF; s[32] = 0x25; s[33] = 0; s[34] = 0; s[35] = 0; s[36] = 0;
	*reinterpret_cast<uint64_t*>(s + 37) = resumeAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, 16);
}

static bool PatchSite7(uintptr_t hookAddr, uint8_t* stubPage, size_t& offset)
{
	const auto* b = reinterpret_cast<const uint8_t*>(hookAddr);
	static const uint8_t kSig[] = { 0x48, 0x8B, 0x90, 0xE8, 0x00, 0x00, 0x00, 0xEB };
	if (std::memcmp(b, kSig, sizeof(kSig)) != 0)
	{
		return false;
	}

	const uintptr_t fallbackAddr = hookAddr + 9;
	const int8_t relResume = *reinterpret_cast<const int8_t*>(hookAddr + 8);
	const uintptr_t resumeAddr = fallbackAddr + static_cast<intptr_t>(relResume);

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC0;                                 // test rax, rax
	s[3] = 0x74; s[4] = 0x15;                                              // jz -> 0x1A
	s[5] = 0x48; s[6] = 0x8B; s[7] = 0x90;                                 // mov rdx, [rax+0xE8]
	s[8] = 0xE8; s[9] = 0x00; s[10] = 0x00; s[11] = 0x00;
	s[12] = 0xFF; s[13] = 0x25; s[14] = 0; s[15] = 0; s[16] = 0; s[17] = 0;
	*reinterpret_cast<uint64_t*>(s + 18) = resumeAddr;
	s[26] = 0xFF; s[27] = 0x25; s[28] = 0; s[29] = 0; s[30] = 0; s[31] = 0;
	*reinterpret_cast<uint64_t*>(s + 32) = fallbackAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, 9);
}

static bool PatchSite8(uintptr_t hookAddr, uint8_t* stubPage, size_t& offset)
{
	const auto* b = reinterpret_cast<const uint8_t*>(hookAddr);
	static const uint8_t kSig[] = {
		0x48, 0x8B, 0x98, 0xE8, 0x00, 0x00, 0x00,
		0x48, 0x89, 0x9D, 0x48, 0x06, 0x00, 0x00,
		0xEB
	};
	if (std::memcmp(b, kSig, sizeof(kSig)) != 0)
	{
		return false;
	}

	const uintptr_t fallbackAddr = hookAddr + 16;
	const int8_t relResume = *reinterpret_cast<const int8_t*>(hookAddr + 15);
	const uintptr_t resumeAddr = fallbackAddr + static_cast<intptr_t>(relResume);

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC0;                                 // test rax, rax
	s[3] = 0x74; s[4] = 0x1C;                                              // jz -> 0x21
	s[5] = 0x48; s[6] = 0x8B; s[7] = 0x98;                                 // mov rbx, [rax+0xE8]
	s[8] = 0xE8; s[9] = 0x00; s[10] = 0x00; s[11] = 0x00;
	s[12] = 0x48; s[13] = 0x89; s[14] = 0x9D;                              // mov [rbp+0x648], rbx
	s[15] = 0x48; s[16] = 0x06; s[17] = 0x00; s[18] = 0x00;
	s[19] = 0xFF; s[20] = 0x25; s[21] = 0; s[22] = 0; s[23] = 0; s[24] = 0;
	*reinterpret_cast<uint64_t*>(s + 25) = resumeAddr;
	s[33] = 0xFF; s[34] = 0x25; s[35] = 0; s[36] = 0; s[37] = 0; s[38] = 0;
	*reinterpret_cast<uint64_t*>(s + 39) = fallbackAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, 16);
}

static bool PatchGenericPatternA(const PatternAMatch& m, uintptr_t dummyDataAddr, uint8_t* stubPage, size_t& offset)
{
	const uintptr_t resumeAddr = m.addr + m.prefixLen;
	const auto* tail = reinterpret_cast<const uint8_t*>(resumeAddr);

	// Skip sites that already validate RAX (`test rax, rax; jz/je`)
	if (tail[0] == 0x48 && tail[1] == 0x85 && tail[2] == 0xC0 &&
		(tail[3] == 0x74 || (tail[3] == 0x0F && tail[4] == 0x84)))
	{
		return false;
	}

	const uintptr_t hookAddr = m.addr + 10;
	const size_t hookLen = m.prefixLen - 10;
	const int32_t relCall = *reinterpret_cast<const int32_t*>(m.addr + 15);
	const uintptr_t callTarget = (m.addr + 19) + static_cast<intptr_t>(relCall);

	uint8_t* s = stubPage + offset;
	s[0] = 0x48; s[1] = 0x85; s[2] = 0xC9;                                 // test rcx, rcx
	s[3] = 0x74; s[4] = 0x25;                                              // jz -> 0x2A
	s[5] = 0x33; s[6] = 0xD2;                                              // xor edx, edx
	s[7] = 0xFF; s[8] = 0x15; s[9] = 0x02; s[10] = 0; s[11] = 0; s[12] = 0;// call [rip+2]
	s[13] = 0xEB; s[14] = 0x08;                                            // jmp +8 -> 0x17
	*reinterpret_cast<uint64_t*>(s + 15) = callTarget;
	s[23] = 0x48; s[24] = 0x85; s[25] = 0xC0;                              // test rax, rax
	s[26] = 0x74; s[27] = 0x0E;                                            // jz -> 0x2A
	s[28] = 0xFF; s[29] = 0x25; s[30] = 0; s[31] = 0; s[32] = 0; s[33] = 0;
	*reinterpret_cast<uint64_t*>(s + 34) = resumeAddr;
	s[42] = 0x48; s[43] = 0xB8;                                            // mov rax, dummyDataAddr
	*reinterpret_cast<uint64_t*>(s + 44) = dummyDataAddr;
	s[52] = 0xFF; s[53] = 0x25; s[54] = 0; s[55] = 0; s[56] = 0; s[57] = 0;
	*reinterpret_cast<uint64_t*>(s + 58) = resumeAddr;
	offset += 96;

	return EmitRelJump(hookAddr, s, hookLen);
}

static bool TryPatchSwitch012Site(uintptr_t switchAddr, uintptr_t dummyF0Addr, uint8_t* stubPage, size_t& offset)
{
	for (size_t back = 14; back <= 26; ++back)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(switchAddr - back);
		if ((p[0] != 0x48 && p[0] != 0x49 && p[0] != 0x4C && p[0] != 0x4D) ||
			p[1] != 0x8B ||
			p[3] != 0xF0 || p[4] != 0x00 || p[5] != 0x00 || p[6] != 0x00)
		{
			continue;
		}
		if ((p[7] != 0x48 && p[7] != 0x49 && p[7] != 0x4D) || p[8] != 0x85 || p[10] != 0x74)
		{
			continue;
		}

		const uint8_t regIdx = static_cast<uint8_t>(((p[2] >> 3) & 7) | ((p[0] & 0x04) ? 8 : 0));
		const uint8_t expectedTestModRM = static_cast<uint8_t>(0xC0 | (regIdx & 7) | ((regIdx & 7) << 3));
		if (p[9] != expectedTestModRM)
		{
			continue;
		}

		const uintptr_t hookAddr = reinterpret_cast<uintptr_t>(p);
		const uintptr_t resumeAddr = hookAddr + 12;

		uint8_t* s = stubPage + offset;
		std::memcpy(s, p, 10);                                             // mov reg, [base+0xF0]; test reg, reg
		s[10] = 0x75; s[11] = 0x0A;                                        // jnz +10 (skip dummy assignment)
		s[12] = (regIdx >= 8) ? 0x49 : 0x48;
		s[13] = static_cast<uint8_t>(0xB8 + (regIdx & 7));                 // mov reg64, dummyF0Addr
		*reinterpret_cast<uint64_t*>(s + 14) = dummyF0Addr;
		s[22] = 0xFF; s[23] = 0x25; s[24] = 0; s[25] = 0; s[26] = 0; s[27] = 0;
		*reinterpret_cast<uint64_t*>(s + 28) = resumeAddr;
		offset += 96;

		return EmitRelJump(hookAddr, s, 12);
	}
	return false;
}
}

static HookFunction hookFunction([]()
{
	if (!xbr::IsGameBuildOrGreater<3889>())
	{
		return;
	}

	std::vector<PatternAMatch> patAMatches;

	auto collectPatternA = [&patAMatches](const char* patStr)
	{
		auto pat = hook::pattern(patStr);
		for (size_t i = 0; i < pat.size(); ++i)
		{
			const auto* p = pat.get(i).get<uint8_t>();
			if (p[20] == 0x03 && p[21] == 0x48 && p[22] == 0x8B && p[23] == 0xC3)
			{
				patAMatches.push_back({ reinterpret_cast<uintptr_t>(p), 24 });
			}
			else if (p[20] == 0x02 && p[21] == 0x33 && p[22] == 0xC0)
			{
				patAMatches.push_back({ reinterpret_cast<uintptr_t>(p), 23 });
			}
		}
	};

	collectPatternA("48 8B ? F0 00 00 00 48 85 C9 74 09 33 D2 E8 ? ? ? ? EB");
	collectPatternA("49 8B ? F0 00 00 00 48 85 C9 74 09 33 D2 E8 ? ? ? ? EB");

	auto swPat = hook::pattern("85 ? 74 14 FF ? 74 0A FF ? 75");

	if (patAMatches.empty() && swPat.size() == 0)
	{
		return;
	}

	const size_t totalNeeded = (patAMatches.size() + swPat.size() + 4) * 96;
	auto* stubPage = static_cast<uint8_t*>(hook::AllocateStubMemory(totalNeeded));
	if (!stubPage)
	{
		return;
	}

	const DummyEnvironment dummy = BuildDummyEnvironment(stubPage);
	size_t stubOffset = 96; // slot 0 holds retZeroFn

	for (const PatternAMatch& m : patAMatches)
	{
		if (stubOffset + 96 > totalNeeded)
		{
			break;
		}

		const uintptr_t hookAddr = m.addr + m.prefixLen;

		if (PatchSite1(hookAddr, stubPage, stubOffset) ||
			PatchSite2(hookAddr, stubPage, stubOffset) ||
			PatchSite3(hookAddr, stubPage, stubOffset) ||
			PatchSite4(hookAddr, stubPage, stubOffset) ||
			PatchSite5(hookAddr, stubPage, stubOffset) ||
			PatchSite6(hookAddr, stubPage, stubOffset) ||
			PatchSite7(hookAddr, stubPage, stubOffset) ||
			PatchSite8(hookAddr, stubPage, stubOffset))
		{
			continue;
		}

		PatchGenericPatternA(m, dummy.dummyDataAddr, stubPage, stubOffset);
	}

	for (size_t i = 0; i < swPat.size(); ++i)
	{
		if (stubOffset + 96 > totalNeeded)
		{
			break;
		}

		const uintptr_t swAddr = reinterpret_cast<uintptr_t>(swPat.get(i).get<uint8_t>());
		TryPatchSwitch012Site(swAddr, dummy.dummyF0Addr, stubPage, stubOffset);
	}

	FlushInstructionCache(GetCurrentProcess(), stubPage, stubOffset);
});
