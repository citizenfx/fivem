#include "StdInc.h"
#include "Hooking.h"

#include <mutex>

static std::unordered_map<void*, std::string> g_gamerInfoBuffer;
static std::mutex g_gamerInfoBufferMutex;

int SprintfToBuffer(char* buffer, size_t length, const char* format, const char* name)
{
	int rv = _snprintf(buffer, length, format, name);
	buffer[length - 1] = '\0';

	std::unique_lock<std::mutex> lock(g_gamerInfoBufferMutex);
	g_gamerInfoBuffer[buffer] = name;

	return rv;
}

static void(*g_origCallGfx)(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7);

static void WrapGfxCall(void* a1, char* a2, void* a3, void* a4, char* a5, void* a6, void* a7)
{
	std::unique_lock<std::mutex> lock(g_gamerInfoBufferMutex);

	// overwrite this argument
	// it's actually a GFx string, however it won't try to free
	*(const char**)(a5 + 16) = g_gamerInfoBuffer[*(char**)(a5 + 16)].c_str();

	g_origCallGfx(a1, a2, a3, a4, a5, a6, a7);
}

#define GAMER_TAG_FOR_NET_PLAYER_LIMIT 256 // !!MUST!! match to the size of g_players
#define GAMER_TAG_FOR_PED_LIMIT 128
#define GAMER_TAG_LIMIT (GAMER_TAG_FOR_NET_PLAYER_LIMIT + GAMER_TAG_FOR_PED_LIMIT)

#pragma pack(push, 1)
struct CGamerTag
{
	char dummy[0x290];
};

struct CGamerInfo
{
	char gamer_info_specific[0x40];
	CGamerTag gamerTag[GAMER_TAG_LIMIT];
	char locker[0x28];
	char *componentNames[30];
	uint32_t componentFlags[30];
};
#pragma pack(pop)

void LimitCheckPatch(uint64_t addr)
{
	uint64_t work_addr = addr;

	hook::adjust_base(work_addr);

	uint8_t opcode = *(uint8_t*)addr;
	bool hasPrefix = false;
	uint8_t prefix = opcode;
	if (opcode != 0x83) // cmp short
	{
		opcode = *(uint8_t*)(addr + 1);
		hasPrefix = true;
		++work_addr;
	}

	/*if (opcode != 0x83) // cmp short
		trace("Fatal error");*/

	++work_addr;

	uint8_t reg = *(uint8_t*)work_addr;
	++work_addr;

	uint32_t size = 0;
	switch (*(uint8_t*)work_addr)
	{
	case 0x20: size = GAMER_TAG_FOR_NET_PLAYER_LIMIT; break;
	case 0x33: size = GAMER_TAG_LIMIT - 1; break;
	case 0x34: size = GAMER_TAG_LIMIT; break;
	}
	++work_addr;

	uint8_t *payload = (uint8_t *)hook::AllocateStubMemory(
		1 + 1 + 1 + 4 +
		1 + 1 + 4 +
		1 + 4
	);

	int i = 0;
	if (hasPrefix)
		payload[i++] = prefix;
	payload[i++] = 0x81; // cmp
	payload[i++] = reg;
	*(uint32_t *)&payload[i] = size; i += prefix == 0x66 ? 2 : 4;

	if (*(uint8_t*)(work_addr) == 0x0F)
		++work_addr;

	uint8_t jump_op = *(uint8_t *) work_addr;
	++work_addr;

	uint64_t dest;
	if ((jump_op & 0xF0) == 0x70) // is short
	{
		dest = work_addr + *(char *)work_addr + 1;
		++work_addr;
	}
	else
	{
		dest = work_addr + *(int *)work_addr + 4;
		work_addr += 4;
	}

	payload[i++] = 0x0F;
	payload[i++] = 0x80 | (jump_op & 0x0F);
	*(uint32_t *)&payload[i] = dest - ((uint64_t)&payload[i] + 4); i += 4;
		 
	payload[i++] = 0xE9;
	*(uint32_t *)&payload[i] = work_addr - ((uint64_t)&payload[i] + 4); i += 4;

	*(uint8_t *)addr = 0xE9;
	*(uint32_t *)(addr + 1) = (uint64_t)payload - (addr + 5);
}

static HookFunction hookFunction([]()
{
	// gamer tag creation
	hook::call(hook::get_pattern("48 8D 8F ? ? 00 00 C6 87 B0 00 00 00 01 E8", 14), SprintfToBuffer);

	// gamer tag name setting
	hook::call(hook::get_pattern("48 8D 8D ? 02 00 00 4C 8B CE", 15), SprintfToBuffer);

	// gamer tag display call
	{
		auto location = hook::get_pattern("E8 ? ? ? ? 44 88 A4 3B F2 00 00 00 8B 84 3B ? 02 00 00 48", 0);
		hook::set_call(&g_origCallGfx, location);
		hook::call(location, WrapGfxCall);
	}

	auto matches = hook::pattern("75 ? ? 10 87 00 00").count(2);

	assert(matches.size() <= 2);

	for (int i = 0; i < matches.size(); i++)
	{
		char* loc = matches.get(i).get<char>(3);
		hook::put<uint32_t>(loc, sizeof(CGamerInfo));
	}
		
	// update locker offset
	uint32_t lockerOffset = offsetof(CGamerInfo, locker);

	matches = hook::pattern("8D ? 80 85 00 00").count(7);

	assert(matches.size() <= 7);

	for (int i = 0; i < matches.size(); i++)
	{
		char* loc = matches.get(i).get<char>(2);
		hook::put<uint32_t>(loc, lockerOffset);
	}

	// update compnent offsets
	uint32_t componentNamesOffset = offsetof(CGamerInfo, componentNames);
	uint32_t componentFlagsOffset = offsetof(CGamerInfo, componentFlags);

	char* locPat1 = hook::pattern("48 ? ? A8 85 00 00").count(1).get(0).get<char>(0);
	char* locPat2 = hook::pattern("44 ? ? 98 86 00 00").count(1).get(0).get<char>(0);

	for (int i = 0; i < 30; i++)
	{
		// GAMER_INFO::GAMER_INFO
		hook::put<uint32_t>(locPat1 + 3 + 14 * i, componentNamesOffset + 8 * i);
		hook::put<uint32_t>(locPat2 + 3 + 7 * i, componentFlagsOffset + 4 * i);
	}

	char* loc = hook::pattern("41 83 FF 1D 0F 87 ? ? ? ? 48 69 FF").count(1).get(0).get<char>(0);

	hook::put<uint32_t>((loc + 0x28) + 4, componentFlagsOffset);
	hook::put<uint32_t>((loc + 0x30) + 4, componentNamesOffset);
	hook::put<uint32_t>((loc + 0x63) + 4, componentFlagsOffset);

	matches = hook::pattern("74 ? ? ? ? 98 86 00 00").count(4);

	assert(matches.size() <= 5);

	for (int i = 0; i < matches.size(); i++)
	{
		loc = matches.get(i).get<char>(5);
		hook::put<uint32_t>(loc, componentFlagsOffset);
	}

	loc = hook::pattern("0F 86 ? ? ? ? 42 8B 84 BB").count(1).get(0).get<char>(0);
	// GAMER_INFO::sub_1401D094C
	loc += 10;
	hook::put<uint32_t>(loc, componentFlagsOffset);
	loc += 8;
	hook::put<uint32_t>(loc, componentNamesOffset);
	loc += 51;
	hook::put<uint32_t>(loc, componentFlagsOffset);

	// GAMER_INFO::sub_1401D0AB0
	hook::put<uint32_t>(hook::get_pattern("48 8D BE ? ? ? ? F6 07 ?", 3), componentFlagsOffset);
	// GAMER_INFO::sub_1401D0B5C
	hook::put<uint32_t>(hook::get_pattern("48 8B B4 F9 ? ? ? ? 24 ?", 4), componentNamesOffset);
	hook::put<uint32_t>(hook::get_pattern("83 8C BB ? ? ? ? ? EB ? 48 8B 4C 24 ?", 3), componentFlagsOffset);
	hook::put<uint32_t>(hook::get_pattern("83 8C BB ? ? ? ? ? EB ? 48 8B CD", 3), componentFlagsOffset);
	hook::put<uint32_t>(hook::get_pattern("83 8C BB ? ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ?", 3), componentFlagsOffset);

	// GAMER_INFO::sub_1401D17B4
	hook::put<uint32_t>(hook::get_pattern("41 F6 84 9E", 4), componentFlagsOffset);
	// GAMER_INFO::sub_1401D1968
	hook::put<uint32_t>(hook::get_pattern("F6 84 9F ? ? ? ? ? 74", 3), componentFlagsOffset);
	// GAMER_INFO::sub_1401D2230
	hook::put<uint32_t>(hook::get_pattern("8B 84 BE ? ? ? ? 45 33 FF", 3), componentFlagsOffset);
	hook::put<uint32_t>(hook::get_pattern("8B 84 BE ? ? ? ? A8", 3), componentFlagsOffset);

	// change limits
	// GAMER_INFO::GAMER_INFO
	{
		uint64_t addr = (uint64_t)hook::get_pattern("8D 6F ? 44 88 79");
		uint8_t payload[] =
		{
			0x8D, 0xAF, 0x00, 0x00, 0x00, 0x00,
			0x44, 0x88, 0x79, 0x0C,
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		uint8_t *stub = (uint8_t *)hook::AllocateStubMemory(sizeof(payload));

		memcpy(stub, payload, sizeof(payload));

		*(uint32_t *)&stub[2] = GAMER_TAG_LIMIT - 1;

		*(uint32_t *)(&stub[11]) = (addr + 3 + 4) - ((uint64_t)&stub[11] + 4);

		*(uint8_t *)addr = 0xE9;
		*(uint32_t *)(addr + 1) = (uint64_t)stub - (addr + 5);
	}

	LimitCheckPatch((uint64_t)hook::get_pattern("66 83 FF 34 7C E0 48 8B C3 48 8B 9C 24 ? ? ? ? 48"));
	// GAMER_TAG::Release
	hook::put<uint32_t>(hook::get_pattern("BE ? ? ? ? 48 8D 99 ? ? ? ? 48 81 EB ? ? ? ? 8B", 1), GAMER_TAG_LIMIT - 1);
	hook::put<uint32_t>(hook::get_pattern("48 8D 99 ? ? ? ? 48 81 EB ? ? ? ? 8B 43", 3), GAMER_TAG_LIMIT * sizeof(CGamerTag) + 0x270);

	hook::put<uint32_t>(hook::get_pattern("BD ? ? ? ? 80 7B ? ? 75", 1), GAMER_TAG_LIMIT); // GAMER_INFO::~GAMER_INFO
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FE ? 0F 8C ? ? ? ? 45 84 E4")); // GAMER_INFO::sub_1401D1358

	// GAMER_INFO::sub_1401D1FA4 changes the tag drawing order by the distence
	hook::put<uint32_t>((uint64_t)hook::get_pattern("48 81 EC ? ? ? ? 8B 15 ? ? ? ? 0F 29 70 D8", 3), 0xB0 + (8 * GAMER_TAG_LIMIT));
	
	{
		uint64_t addr = (uint64_t)hook::get_pattern("41 8D 4E ? F3 0F 10 18");
		uint8_t payload[] =
		{
			0x41, 0x8D, 0x8E, 0x00, 0x00, 0x00, 0x00,
			0xF3, 0x0F, 0x10, 0x18,
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		uint8_t *stub = (uint8_t *)hook::AllocateStubMemory(sizeof(payload));

		memcpy(stub, payload, sizeof(payload));

		*(uint32_t *)&stub[3] = GAMER_TAG_LIMIT - 2;

		*(uint32_t *)(&stub[12]) = (addr + 4 + 4) - ((uint64_t)&stub[12] + 4);

		*(uint8_t *)addr = 0xE9;
		*(uint32_t *)(addr + 1) = (uint64_t)stub - (addr + 5);
	}

	LimitCheckPatch((uint64_t)hook::get_pattern("66 83 FA ? 0F 8C"));
	hook::put<uint32_t>(hook::get_pattern("E8 ? ? ? ? 66 41 03 DE 0F BF D3 3B 95", (24 + 4)), 0xB0 + (8 * GAMER_TAG_LIMIT));

	// GAMER_INFO::createGamerTagForPed
	hook::put<uint32_t>(hook::get_pattern("BB ? ? ? ? 41 B8 ? ? ? ? 75", 1), GAMER_TAG_FOR_NET_PLAYER_LIMIT);
	LimitCheckPatch((uint64_t)hook::get_pattern("48 83 FA ? 7C ? 48 8B CB"));
	LimitCheckPatch((uint64_t)hook::get_pattern("48 83 F9 ? 7C ? 83 C8 ? 48 8B 5C 24"));

	matches = hook::pattern("48 8D 87 ? ? ? ? 80 78 ? ? 75 ? 80 38 ? 75 ? 80 78 ? ? 75 ? 80 78 ? ? 74 ? 48").count(2);

	assert(matches.size() <= 2);

	for (int i = 0; i < matches.size(); i++)
	{
		loc = matches.get(i).get<char>(3);
		hook::put<uint32_t>(loc, 0x40 + sizeof(CGamerTag) * GAMER_TAG_FOR_NET_PLAYER_LIMIT + 0xC7);
	}

	LimitCheckPatch((uint64_t)hook::get_pattern("83 FE 33 0F ? ? ? ? ? 48", 0)); // GAMER_INFO::createGamerTag
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FB 34 7C D8 83 C8 FF")); // GAMER_INFO::gamerTagOfLocalPed
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FA ? 7D ? 8A CA")); // GAMER_INFO::gamerTagGetPed
	LimitCheckPatch((uint64_t)hook::get_pattern("E8 ? ? ? ? 84 C0 74 23 83 FE 33", 9)); // IS_MP_GAMER_TAG_ACTIVE

	LimitCheckPatch((uint64_t)hook::get_pattern("83 FE 33 77 30 48 8B C6 48 69 C0 ? ? ? ? 38")); // GAMER_INFO::gamerTagAddTrevorRandomModifier

	matches = hook::pattern("83 FB ? 77 ? 48 69 DB").count(2); // GAMER_INFO::gamerTagIsCrewConfigSet // GAMER_INFO::gamerTagSetCrew

	assert(matches.size() <= 2);

	for (int i = 0; i < matches.size(); i++)
	{
		uint64_t off = (uint64_t)matches.get(i).get<uint64_t>(0);
		LimitCheckPatch(off);
	}

	LimitCheckPatch((uint64_t)hook::get_pattern("83 FB ? 77 ? 48 8B C3 48 69 C0")); // GAMER_INFO::gamerTagIsCrewConfigSet
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FB ? 77 ? 48 8B FB")); // GAMER_INFO::gamerTagRemove
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FF ? 77 ? 48 8B CF")); // GAMER_INFO::gamerTagSetHealthBarColour
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FB ? 0F 8C ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 41")); // GAMER_INFO::update //140208578
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FF ? 0F 87 ? ? ? ? 4C 8B CF")); // GAMER_INFO::gamerTagUnknown
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FE ? 0F 87 ? ? ? ? 83 FF")); // GAMER_INFO::gamerTagSetAlpha
	LimitCheckPatch((uint64_t)hook::get_pattern("38 8C 18 ? ? ? ? 75 26 48 8D 8B", -86)); // GAMER_INFO::gamerTagSetGamerInfoSTR
	LimitCheckPatch((uint64_t)hook::get_pattern("0F 84 ? ? ? ? 8D 47 FF 83 F8 01 0F", 18)); // GAMER_INFO::gamerTagSetColour
	LimitCheckPatch((uint64_t)hook::get_pattern("39 ? ? 68 01 00 00 74 41", -58)); // GAMER_INFO::gamerTagSetLevel
	LimitCheckPatch((uint64_t)hook::get_pattern("83 FD ? 0F 87 ? ? ? ? 83")); // GAMER_INFO::gamerTagSetVisibility
	LimitCheckPatch((uint64_t)hook::get_pattern("74 ? 38 8C 18 ? ? ? ? 74 ?", -31)); // GAMER_INFO::gamerTagNewNative
	LimitCheckPatch((uint64_t)hook::get_pattern("74 ? 40 88 BC 18 ? ? ? ? 40 84 FF", -45)); // GAMER_INFO::gamerTagNewNative2
	LimitCheckPatch((uint64_t)hook::get_pattern("74 ? 40 88 BC 18 ? ? ? ? 48 8B 5C 24", -45)); // GAMER_INFO::gamerTagSetIcons

	hook::nop(hook::get_pattern("48 8B CB E8 ? ? ? ? 84 C0 74 ? F6 83 ? ? ? ? ? 75 ? 48 8B CB", 0), 12); // GAMER_INFO::gamerTagGetPed onesync fix
});
