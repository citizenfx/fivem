#include "StdInc.h"
#include "Hooking.h"

#include <CrossBuildRuntime.h>
#include <mutex>
#include <Error.h>

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

struct GamerTag
{
public:
    struct Impl505
    {
        char dummy[0x290];
    };
 
    struct Impl2060
    {
        char dummy[0x2A0];
    };
 
    union
    {
        Impl505 m1604;
        Impl2060 m2060;
    } impl;
};
 
template<int Build>
using CGamerTag = std::conditional_t<(Build >= 2060), GamerTag::Impl2060, GamerTag::Impl505>;

template <int Build>
struct CGamerInfo
{
	char gamer_info_specific[0x40];
	CGamerTag<Build> gamerTag[GAMER_TAG_LIMIT];
	char locker[0x28];
	char *componentNames[30];
	uint32_t componentFlags[30];
};
#pragma pack(pop)

// This function is used to patch the hard-coded gamertag limits.
// Since these limits are normally 32/20, the limit is stored as a byte internally by the game.
// We use a stub/hook on each byte `cmp` to increase the size to at least 16-bit
// The address passed into this function should be the start of a cmp line, which will start as 0x83 or a 1-byte Prefix like 0x41/0x66
void LimitPatch(void *address)
{
	const uint8_t* const addrStart = (uint8_t*)address;
	uint8_t* addr = (uint8_t*)address;

	hook::adjust_base(addr);

	uint8_t opcode = *addrStart;
	bool hasPrefix = false;
	uint8_t prefix = opcode;

	if (opcode != 0x83) // cmp r/m16/32 imm8
	{
		opcode = *(addrStart + 1);
		hasPrefix = true;
		addr++;
	}

	addr++;

	uint8_t reg = *addr;
	addr++;

	// Note: original opcode 0x83 specifies `r/m16/32 	imm8`
	//       Our new opcode 0x81 specifies  `r/m16/32 	imm16/32`
	// 32-bit registers are NOT always the case, but we are guaranteed at least 16-bit.
	uint16_t newSize = 0;
	switch (*addr)
	{
		case 0x20: newSize = GAMER_TAG_FOR_NET_PLAYER_LIMIT; break;
		case 0x33: newSize = GAMER_TAG_LIMIT - 1; break;
		case 0x34: newSize = GAMER_TAG_LIMIT; break;
		default:
			FatalError("Broken Sig!!\n");
			break;
	}
	addr++;

	uint8_t *payload = (uint8_t *)hook::AllocateStubMemory(
		1 + 1 + 1 + 4 + // [prefix] [cmp] [reg] [imm16/32]
		1 + 1 + 4 +		// [jmp-F0] [jmp-compare-logic] [offset32]
		1 + 4			// [jmp-E9] [offset32]
	);

	int i = 0;

	// Start building the stub... redo the original cmp, but make it imm16 instead of imm8
	if (hasPrefix)
	{
		payload[i++] = prefix;	
	}
	payload[i++] = 0x81; // cmp reg imm16/imm32
	payload[i++] = reg;
	// If the prefix is 0x66, it's a 16-bit register and we need an imm16
	if ( prefix == 0x66 )
	{
		*(uint16_t*)&payload[i] = newSize;
		i += 2;
	}
	else
	{
		*(uint32_t*)&payload[i] = (uint32_t)newSize;
		i += 4;
	}

	// 2-byte opcode 
	if (*addr == 0x0F)
	{
		addr++;
	}

	uint8_t jump_op = *addr;
	addr++;

	uint64_t dest;
	if ((jump_op & 0xF0) == 0x70) // is short jmp
	{
		dest = (uint64_t)addr + *(int8_t*)addr + 1;
		addr++;
	}
	else
	{
		dest = (uint64_t)addr + *(int32_t*)addr + 4;
		addr += 4;
	}

	// In the Stub, emulate the jmp that is after the cmp in the original code. (This is a possible exit-point from the stub.)
	// 0F 8? XX XX XX XX ( 6 bytes )
	payload[i++] = 0x0F;					// 2 byte opcode
	payload[i++] = 0x80 | (jump_op & 0x0F); // jmp long + logic options from the original code
	*(uint32_t *)&payload[i] = dest - ((uint64_t)&payload[i] + 4);
	i += 4;

	// Finish the stub with a return call back into the original code
	// E9 XX XX XX XX ( 5 bytes )
	payload[i++] = 0xE9;
	*(uint32_t *)&payload[i] = (uint64_t)addr - ((uint64_t)&payload[i] + 4);

	// replace the original code with a call to our Stub
	// E9 XX XX XX XX ( 5 bytes required in the original code )
	hook::put<uint8_t>(addrStart, 0xE9);
	hook::put<uint32_t>(addrStart + 1, (uint64_t)payload - (uint64_t)(addrStart + 5));
}

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

static HookFunction hookFunction([]()
{
	const size_t gamerInfoSize = (xbr::IsGameBuildOrGreater<2060>()) ? sizeof(CGamerInfo<2060>) : sizeof(CGamerInfo<1604>);
	const size_t gamerTagSize = (xbr::IsGameBuildOrGreater<2060>()) ? sizeof(CGamerTag<2060>) : sizeof(CGamerTag<1604>);
	const uint32_t lockerOffset = (xbr::IsGameBuildOrGreater<2060>()) ? offsetof(CGamerInfo<2060>, locker) : offsetof(CGamerInfo<1604>, locker); // [2189] 0x88C0
	const uint32_t componentNamesOffset = (xbr::IsGameBuildOrGreater<2060>()) ? offsetof(CGamerInfo<2060>, componentNames) : offsetof(CGamerInfo<1604>, componentNames); // [2189] 0x88E8
	const uint32_t componentFlagsOffset = (xbr::IsGameBuildOrGreater<2060>()) ? offsetof(CGamerInfo<2060>, componentFlags) : offsetof(CGamerInfo<1604>, componentFlags); // [2189] 0x89D8

	// gamer tag creation
	hook::call(hook::get_pattern("48 8D 8F ? ? 00 00 C6 87 ? 00 00 00 01 E8 ? ? ? ? 48 8D", 14), SprintfToBuffer);

	// gamer tag name setting
	hook::call(hook::get_pattern("48 8D 8D ? 02 00 00 4C 8B CE", 15), SprintfToBuffer);

	// gamer tag display call
	{
		auto location = hook::get_pattern("E8 ? ? ? ? 44 88 A4 3B ? ? 00 00 8B 84 3B ? 02 00 00 48", 0);
		hook::set_call(&g_origCallGfx, location);
		hook::call(location, WrapGfxCall);
	}

	//xref "CPauseMenu::CloseInternal", to go top of function, should see a big # being passed to a malloc() wrapper.
	//search immediate to find the other one.
	//size passed into malloc() 1604(0x8710) - 2060/2189(0x8A50)
	std::initializer_list<PatternPair> gamerInfoSizeMallocs = {
		{ "8B 15 ? ? ? ? 33 C9 E8 ? ? ? ? 40", 32 },
		{ (xbr::IsGameBuildOrGreater<2802>() ? "E9 ? ? ? ? 53 48 83 EC 20 48 83 3D" : "83 F9 08 75 ? 53 48 83 EC 20 48 83 3D ? ? ? ? 00 75"), 21 },
	};
	for (auto& entry : gamerInfoSizeMallocs)
	{
		auto loc = hook::pattern(entry.pattern).count(1).get(0).get<char>(entry.offset);
		hook::put<uint32_t>(loc, gamerInfoSize);
	}
	

	// xref "GAMER_NAME", go to the very top of the asm block, look for a big offset (0x88C0)
	// search immediate to find the others, there should be 7 total
	std::initializer_list<PatternPair> lockerOffsets = {
		{ "48 81 C6 ? ? ? ? 41 2B EF 79 ? 48 8D 8B", 15 },
		{ "48 8B F9 49 8B CF E8 ? ? ? ? 4C", -4 },
		{ "84 C0 0F 84 ? ? ? ? 33 C9 45 32 E4 E8", -20 },
		{ "4C 8D 05 ? ? ? ? 44 8B CE BA ? ? ? ? E8", 38 },
		{ "8B 73 04 8B CE E8 ? ? ? ? 8B 53 04 48", -12 },
		{ "C6 87 ? ? ? ? 01 66 C7 87 ? ? ? ? 00", -12 },
		{ "F6 04 01 01 0F 84 ? ? ? ? 48 8D B3", 13 },
	};
	for (auto& entry : lockerOffsets)
	{
		auto loc = hook::pattern(entry.pattern).count(1).get(0).get<char>(entry.offset);
		hook::put<uint32_t>(loc, lockerOffset);
	}

	//In the GAMER_INFO constructor
	auto location = hook::pattern("4C 8D 05 ? ? ? ? 48 8D 4B 04 44 88 7C 24 28").count(1).get(0);
	//right below "GAMER_NAME"
	char* locPat1 = location.get<char>(0x37);
	//2 below "MP_BOMB", 1st in the r15d spam
	char* locPat2 = location.get<char>(0x1D4);

	for (int i = 0; i < 30; i++)
	{
		// Basically there is a series of 30 string sets into this struct.
		// 48 8D 05 C1 45 79 01                    lea     rax, aGamerName ; "GAMER_NAME"
		// 48 89 83 E8 88 00 00                    mov     [rbx+88E8h], rax   <----------- [Start]
		// 48 8D 05 C3 45 79 01                    lea     rax, aCrewTag   ; "CREW_TAG"
		// 48 89 83 F0 88 00 00					   mov     [rbx+88F0h], rax	
		//.......
		hook::put<uint32_t>(locPat1 + (14 * i) + 3, componentNamesOffset + (sizeof(char*) * i));

		// Later on in the function, there is also a series of 30 DWORD flags for each string.
		// 44 89 BB D8 89 00 00                    mov     [rbx+89D8h], r15d  <----------- [Start]
        // 44 89 BB DC 89 00 00                    mov     [rbx+89DCh], r15d
        // 44 89 BB E0 89 00 00                    mov     [rbx+89E0h], r15d
		//.......
		hook::put<uint32_t>(locPat2 + (7 * i) + 3, componentFlagsOffset + (sizeof(uint32_t) * i));
	}

	std::initializer_list<PatternPair> compFlagsList = {
		{ "33 DB 84 C0 74 ? 48 8D BE", 9 },
		{ "3C ? 4C 8B C6 41 0F 94 C1 E8", 21 },
		{ "48 8B 09 24 ? 3C ? 41 0F 94 C1 E8", 23 },
		{ "48 8B 8C 19 ? ? ? ? 24 ? 3C ? 41", 28 },
		{ "41 F6 84 9E", 4 },
		{ "F6 84 9F ? ? ? ? ? 74", 3 },
		{ "8B 84 BE ? ? ? ? 45 33 FF", 3 },
		{ "8B 84 BE ? ? ? ? A8", 3 },
		{ "42 8B 84 BB ? ? ? ? 4A", 4 },
		{ "42 8B 84 BB ? ? ? ? B9", 4 }
	};
	for (auto& entry : compFlagsList)
	{
		auto location = hook::pattern(entry.pattern).count(1).get(0).get<char>(entry.offset);
		hook::put<uint32_t>(location, componentFlagsOffset);
	}

	hook::put<uint32_t>(hook::get_pattern("4A 8B AC FB ? ? ? ? 4C", 4), componentNamesOffset);
	hook::put<uint32_t>(hook::get_pattern("48 8B B4 F9 ? ? ? ? 24", 4), componentNamesOffset);

	// change limits
	// GAMER_INFO::GAMER_INFO()
	{
		// the whole point of this is to change the opcodes to support a 4-byte instead of a 1-byte immediate
		uint64_t addr;
		uint8_t payload[] = {
			// 8D AF 00 00 00 00       lea    ebp,[rdi+0x0]
			// 00 00 00 00             [4 BYTE PLACEHOLDER for nearby-opcodes that get overwritten with our jmp hook]
			// E9 00 00 00 00          jmp    0xf
			0x8D, 0xAF, /*stub[2]*/ 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, /*stub[6-9]*/
			0xE9, /*stub[11]*/ 0x00, 0x00, 0x00, 0x00
		};

		// 2372 compiler changed the function a little bit
		if (xbr::IsGameBuildOrGreater<2372>())
		{
			addr = (uint64_t)hook::get_pattern("48 8D 73 40 8D 6F 33");
			// 48 8D 73 40             lea     rsi, [rbx+40h]  <---------
			// 8D 6F 33                lea     ebp, [rdi+51]
			payload[6] = 0x48;
			payload[7] = 0x8D;
			payload[8] = 0x73;
			payload[9] = 0x40;
		}
		else
		{
			addr = (uint64_t)hook::get_pattern("8D 6F ? 44 88 79");
			// 8D 6F 33                    lea     ebp, [rdi+51]
			// 44 88 79 0C                 mov     [rcx+0Ch], r15b  <---------
			payload[6] = 0x44;
			payload[7] = 0x88;
			payload[8] = 0x79;
			payload[9] = 0x0C;
		}

		uint8_t* stub = (uint8_t*)hook::AllocateStubMemory(sizeof(payload));

		memcpy(stub, payload, sizeof(payload));

		*(uint32_t*)&stub[2] = GAMER_TAG_LIMIT - 1;

		*(uint32_t*)(&stub[11]) = (addr + 3 + 4 /*(Both Game-Versions are 7-bytes)*/) - ((uint64_t)&stub[11] + 4);

		// ensure lines are fully NOP'd (Both Game-Versions are 7-bytes)
		hook::nop(addr, 7);

		hook::put<uint8_t>(addr, 0xE9);
		hook::put<uint32_t>(addr + 1, (uint64_t)stub - (addr + 5));
	}

	LimitPatch(hook::get_pattern("66 83 FF 34 7C E0 48 8B C3 48 8B 9C 24 ? ? ? ? 48"));
	// GAMER_TAG::Release
	hook::put<uint32_t>(hook::get_pattern("BE ? ? ? ? 48 8D 99 ? ? ? ? 48 81 EB ? ? ? ? 8B", 1), GAMER_TAG_LIMIT - 1);

	// This magic # comes from the code; basically it goes over the GamerTag Array backwards and takes this member ( EndofArray - (sizeof(gamertag) * i) - 0x288 )
	uint32_t magicOffset = (xbr::IsGameBuildOrGreater<2060>()) ? magicOffset = 0x288 : magicOffset = 0x270;
	hook::put<uint32_t>(hook::get_pattern("48 8D 99 ? ? ? ? 48 81 EB ? ? ? ? 8B 43", 3), GAMER_TAG_LIMIT * gamerTagSize + magicOffset);	


	hook::put<uint32_t>(hook::get_pattern("BD ? ? ? ? 80 7B ? ? 75", 1), GAMER_TAG_LIMIT); // GAMER_INFO::~GAMER_INFO
	LimitPatch(hook::get_pattern("83 FE ? 0F 8C ? ? ? ? 45 84 E4")); // GAMER_INFO::sub_1401D1358

	// GAMER_INFO::sub_1401D1FA4 changes the tag drawing order by the distance
	hook::put<uint32_t>((uint64_t)hook::get_pattern("48 81 EC ? ? ? ? 8B 15 ? ? ? ? 0F 29 70 D8", 3), 0xB0 + (8 * GAMER_TAG_LIMIT));
	
	{
		uint64_t addr = (uint64_t)hook::get_pattern("41 8D 4E ? F3 0F 10 18");
		uint8_t payload[] =
		{
			// 41 8D 8E 00 00 00 00    lea    ecx,[r14+0x0]
			// F3 0F 10 18             movss  xmm3,DWORD PTR [rax]
			// E9 00 00 00 00          jmp    0x10 
			0x41, 0x8D, 0x8E,/*stub[3]*/ 0x00, 0x00, 0x00, 0x00,
			0xF3, 0x0F, 0x10, 0x18,
			0xE9,/*stub[12]*/ 0x00, 0x00, 0x00, 0x00
		};

		uint8_t *stub = (uint8_t *)hook::AllocateStubMemory(sizeof(payload));

		memcpy(stub, payload, sizeof(payload));

		*(uint32_t *)&stub[3] = GAMER_TAG_LIMIT - 2;

		*(uint32_t *)(&stub[12]) = (addr + 4 + 4) - ((uint64_t)&stub[12] + 4);

		hook::put<uint8_t>(addr, 0xE9);
		hook::put<uint32_t>(addr + 1, (uint64_t)stub - (addr + 5));
	}

	LimitPatch(hook::get_pattern("66 83 FA ? 0F 8C"));
	hook::put<uint32_t>(hook::get_pattern("E8 ? ? ? ? 66 41 03 DE 0F BF D3 3B 95", 28), 0xB0 + (8 * GAMER_TAG_LIMIT));

	// GAMER_INFO::createGamerTagForPed
	hook::put<uint32_t>(hook::get_pattern("BB ? ? ? ? 41 B8 ? ? ? ? 75", 1), GAMER_TAG_FOR_NET_PLAYER_LIMIT);
	LimitPatch(hook::get_pattern("48 83 FA ? 7C ? 48 8B CB"));
	LimitPatch(hook::get_pattern("48 83 F9 ? 7C ? 83 C8 ? 48 8B 5C 24"));

	auto matches = hook::pattern("48 8D 87 ? ? ? ? 80 78 ? ? 75 ? 80 38 ? 75 ? 80 78 ? ? 75 ? 80 78 ? ? 74 ? 48").count(2);
	for (int i = 0; i < matches.size(); i++)
	{
		// Another magic offset - this is another struct member
		uint32_t magicOffset2 = (xbr::IsGameBuildOrGreater<2060>()) ? 0xDF : 0xC7;
		auto loc = matches.get(i).get<char>(3);
		hook::put<uint32_t>(loc, 0x40 + gamerTagSize * GAMER_TAG_FOR_NET_PLAYER_LIMIT + magicOffset2);
	}

	LimitPatch(hook::get_pattern("83 FE 33 0F ? ? ? ? ? 48")); // GAMER_INFO::createGamerTag
	LimitPatch(hook::get_pattern("83 FB 34 7C D8 83 C8 FF")); // GAMER_INFO::gamerTagIndexOfLocalPed
	LimitPatch(hook::get_pattern("83 FA ? 7D ? 8A CA")); // GAMER_INFO::gamerTagGetPed
	LimitPatch(hook::get_pattern("E8 ? ? ? ? 84 C0 74 23 83 FE 33", 9)); // IS_MP_GAMER_TAG_ACTIVE
	LimitPatch(hook::get_pattern("83 FE 33 77 30 48 8B C6 48 69 C0 ? ? ? ? 38")); // GAMER_INFO::gamerTagAddTrevorRandomModifier

	matches = hook::pattern("83 FB ? 77 ? 48 69 DB").count(2); // GAMER_INFO::gamerTagIsCrewConfigSet // GAMER_INFO::gamerTagSetCrew
	LimitPatch(matches.get(0).get<void>(0));
	LimitPatch(matches.get(1).get<void>(0));

	LimitPatch(hook::get_pattern("7E 83 FB ? 77 ? 48 8B FB", 1)); // GAMER_INFO::gamerTagRemove
	LimitPatch(hook::get_pattern("83 FF ? 77 ? 48 8B CF")); // GAMER_INFO::gamerTagSetHealthBarColour
	if (xbr::IsGameBuildOrGreater<2060>())
	{
		LimitPatch(hook::get_pattern("83 ? 34 0F 8C ? ? ? ? 4C 8D 9C", -1)); // GAMER_INFO::update	
	}
	else
	{
		LimitPatch(hook::get_pattern("83 ? 34 0F 8C ? ? ? ? 4C 8D 9C")); // GAMER_INFO::update //140208578	
	}
	LimitPatch(hook::get_pattern("83 FF ? 0F 87 ? ? ? ? 4C 8B CF")); // GAMER_INFO::gamerTagUnknown
	LimitPatch(hook::get_pattern("83 FE ? 0F 87 ? ? ? ? 83 FF")); // GAMER_INFO::gamerTagSetAlpha
	LimitPatch(hook::get_pattern("38 8C 18 ? ? ? ? 75 26 48 8D 8B", -0x21)); // GAMER_INFO::gamerTagSetGamerInfoSTR
	LimitPatch(hook::get_pattern("0F 84 ? ? ? ? 8D 47 FF 83 F8 01 0F", 18)); // GAMER_INFO::gamerTagSetColour
	LimitPatch(hook::get_pattern("0F 84 ? ? ? ? 83 FE 33 77 ? 83 FF 1E", 6)); // GAMER_INFO::gamerTagSetLevel
	LimitPatch(hook::get_pattern("83 FD ? 0F 87 ? ? ? ? 83")); // GAMER_INFO::gamerTagSetVisibility
	LimitPatch(hook::get_pattern("74 ? 38 8C 18 ? ? ? ? 74", -31)); // GAMER_INFO::gamerTagNewNative
	LimitPatch(hook::get_pattern("74 ? 40 88 BC 18 ? ? ? ? 40 84 FF", -43)); // GAMER_INFO::gamerTagNewNative2
	LimitPatch(hook::get_pattern("74 ? 40 88 BC 18 ? ? ? ? 48 8B 5C 24", -43)); // GAMER_INFO::gamerTagSetIcons

	hook::nop(hook::get_pattern("48 8B CB E8 ? ? ? ? 84 C0 74 ? F6 83 ? ? ? ? ? 75 ? 48 8B CB"), 12); // GAMER_INFO::gamerTagGetPed onesync fix
});
