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

	if (opcode != 0x83) // cmp short
		trace("Fatal error");

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

	// Addresses are for 1604

	// update size
	// CPauseMenu::CloseInternal
	hook::put<uint32_t>(0x140179599 + 1, sizeof(CGamerInfo));
	// _gamerInfoMenu_initSession
	hook::put<uint32_t>(0x1401B2D08 + 1, sizeof(CGamerInfo));

	// update locker offset
	uint32_t lockerOffset = offsetof(CGamerInfo, locker);

	// GAMER_INFO::GAMER_INFO
	hook::put<uint32_t>(0x140157648 + 3, lockerOffset);
	// GAMER_INFO::~GAMER_INFO
	hook::put<uint32_t>(0x14015BD40 + 3, lockerOffset);
	// GAMER_INFO::sub_1401D1358
	hook::put<uint32_t>(0x1401D13A0 + 3, lockerOffset);
	// GAMER_INFO::createGamerTag
	hook::put<uint32_t>(0x140206DCD + 3, lockerOffset);
	// GAMER_INFO::createMovieClip
	hook::put<uint32_t>(0x140206E4A + 3, lockerOffset);
	// GAMER_INFO::gamerTagRemove
	hook::put<uint32_t>(0x14020727E + 3, lockerOffset);
	// GAMER_INFO::removeMovieClip
	hook::put<uint32_t>(0x1402072FD + 3, lockerOffset);

	// update compnent offsets
	uint32_t componentNamesOffset = offsetof(CGamerInfo, componentNames);
	uint32_t componentFlagsOffset = offsetof(CGamerInfo, componentFlags);

	for (int i = 0; i < 30; i++)
	{
		// GAMER_INFO::GAMER_INFO
		hook::put<uint32_t>(0x1401576CB + 3 + 14 * i, componentNamesOffset + 8 * i);
		hook::put<uint32_t>(0x140157868 + 3 + 7 * i, componentFlagsOffset + 4 * i);
	}

	// GAMER_INFO::sub_1401D094C
	hook::put<uint32_t>(0x1401D09E3 + 4, componentFlagsOffset);
	hook::put<uint32_t>(0x1401D09EB + 4, componentNamesOffset);
	hook::put<uint32_t>(0x1401D0A1E + 4, componentFlagsOffset);
	// GAMER_INFO::sub_1401D0AB0
	hook::put<uint32_t>(0x1401D0B0D + 3, componentFlagsOffset);
	// GAMER_INFO::sub_1401D0B5C
	hook::put<uint32_t>(0x1401D0B7A + 4, componentNamesOffset);
	hook::put<uint32_t>(0x1401D0BA3 + 3, componentFlagsOffset);
	hook::put<uint32_t>(0x1401D0BD0 + 3, componentFlagsOffset);
	hook::put<uint32_t>(0x1401D0C0F + 3, componentFlagsOffset);
	// GAMER_INFO::sub_1401D17B4
	hook::put<uint32_t>(0x1401D1832 + 4, componentFlagsOffset);
	// GAMER_INFO::sub_1401D1968
	hook::put<uint32_t>(0x1401D19D0 + 3, componentFlagsOffset);
	// GAMER_INFO::sub_1401D2230
	hook::put<uint32_t>(0x1401D265C + 3, componentFlagsOffset);
	hook::put<uint32_t>(0x1401D276F + 3, componentFlagsOffset);

	// change limits
	// GAMER_INFO::GAMER_INFO
	{
		uint8_t payload[] =
		{
			0x8D, 0xAF, 0x00, 0x00, 0x00, 0x00,
			0x44, 0x88, 0x79, 0x0C,
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		uint8_t *stub = (uint8_t *)hook::AllocateStubMemory(sizeof(payload));

		memcpy(stub, payload, sizeof(payload));

		*(uint32_t *)&stub[2] = GAMER_TAG_LIMIT - 1;

		*(uint32_t *)(&stub[11]) = (0x14015761F + 3 + 4) - ((uint64_t)&stub[11] + 4);

		*(uint8_t *)0x14015761F = 0xE9;
		*(uint32_t *)(0x14015761F + 1) = (uint64_t)stub - (0x14015761F + 5);
	}

	LimitCheckPatch(0x14015795B);
	// GAMER_TAG::Release
	hook::put<uint32_t>(0x14015B36B + 1, GAMER_TAG_LIMIT - 1);
	hook::put<uint32_t>(0x14015B370 + 3, GAMER_TAG_LIMIT * sizeof(CGamerTag) + 0x270);

	hook::put<uint32_t>(0x14015BD6C + 1, GAMER_TAG_LIMIT); // GAMER_INFO::~GAMER_INFO
	LimitCheckPatch(0x1401D156D); // GAMER_INFO::sub_1401D1358

	// GAMER_INFO::sub_1401D1FA4 changes the tag drawing order by the distence
	hook::put<uint32_t>(0x1401D1FBA + 3, 0xB0 + (8 * GAMER_TAG_LIMIT));
	{
		uint8_t payload[] =
		{
			0x41, 0x8D, 0x8E, 0x00, 0x00, 0x00, 0x00,
			0xF3, 0x0F, 0x10, 0x18,
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		uint8_t *stub = (uint8_t *)hook::AllocateStubMemory(sizeof(payload));

		memcpy(stub, payload, sizeof(payload));

		*(uint32_t *)&stub[3] = GAMER_TAG_LIMIT - 2;

		*(uint32_t *)(&stub[12]) = (0x1401D2016 + 4 + 4) - ((uint64_t)&stub[12] + 4);

		*(uint8_t *)0x1401D2016 = 0xE9;
		*(uint32_t *)(0x1401D2016 + 1) = (uint64_t)stub - (0x1401D2016 + 5);
	}
	LimitCheckPatch(0x1401D20E1);
	hook::put<uint32_t>(0x01401D2209 + 4, 0xB0 + (8 * GAMER_TAG_LIMIT));

	// GAMER_INFO::createGamerTagForPed
	hook::put<uint32_t>(0x140206AF3 + 1, GAMER_TAG_FOR_NET_PLAYER_LIMIT);
	LimitCheckPatch(0x140206B33);
	LimitCheckPatch(0x140206B62);
	hook::put<uint32_t>(0x140206B3C + 3, 0x40 + sizeof(CGamerTag) * GAMER_TAG_FOR_NET_PLAYER_LIMIT + 0xC7);
	hook::put<uint32_t>(0x140206B04 + 3, 0x40 + sizeof(CGamerTag) * GAMER_TAG_FOR_NET_PLAYER_LIMIT + 0xC7);

	LimitCheckPatch(0x140206CAE); // GAMER_INFO::createGamerTag
	LimitCheckPatch(0x140206F9B); // GAMER_INFO::gamerTagOfLocalPed
	LimitCheckPatch(0x140206FE9); // GAMER_INFO::gamerTagGetPed
	LimitCheckPatch(0x1402070A7); // IS_MP_GAMER_TAG_ACTIVE
	LimitCheckPatch(0x14020712B); // GAMER_INFO::gamerTagAddTrevorRandomModifier
	LimitCheckPatch(0x1402071B4); // GAMER_INFO::gamerTagIsCrewConfigSet
	LimitCheckPatch(0x140207236); // GAMER_INFO::gamerTagRemove
	LimitCheckPatch(0x140207458); // GAMER_INFO::gamerTagUnknown
	LimitCheckPatch(0x140207556); // GAMER_INFO::gamerTagSetGamerInfoSTR
	LimitCheckPatch(0x14020760A); // GAMER_INFO::gamerTagSetName
	LimitCheckPatch(0x1402076B8); // GAMER_INFO::gamerTagSetCrew
	LimitCheckPatch(0x14020774C); // GAMER_INFO::gamerTagSetHealthBarColour
	LimitCheckPatch(0x140207808); // GAMER_INFO::gamerTagSetAlpha
	LimitCheckPatch(0x14020795C); // GAMER_INFO::gamerTagSetColour
	LimitCheckPatch(0x140207A54); // GAMER_INFO::gamerTagSetLevel
	LimitCheckPatch(0x140207B4C); // GAMER_INFO::gamerTagSetVisibility
	LimitCheckPatch(0x140207C62); // GAMER_INFO::gamerTagNewNative
	LimitCheckPatch(0x140207CF8); // GAMER_INFO::gamerTagNewNative2
	LimitCheckPatch(0x140207D9C); // GAMER_INFO::gamerTagSetIcons
	LimitCheckPatch(0x140208578); // GAMER_INFO::update
});
