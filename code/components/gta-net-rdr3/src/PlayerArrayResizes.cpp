#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <MinHook.h>
#include <NetLibrary.h>

#include <GameInit.h>
#include <DirectXMath.h>

#include <CoreNetworking.h>
#include <CrossBuildRuntime.h>
#include <Error.h>
#include <CloneManager.h>
#include <xmmintrin.h>
#include <netPlayerManager.h>
#include <netObjectMgr.h>
#include <netObject.h>
#include <CoreConsole.h>

#include <udis86.h>

//#define VERBOSE
static size_t GetDisplacementOffset(const ud_t& ud, ud_mnemonic_code mnemonic, uint64_t stackSize, uint8_t dataSize)
{
	const uint8_t* instr = ud_insn_ptr(&ud);
	size_t len = ud_insn_len(&ud);

	if (!instr || len == 0 || (dataSize != 1 && dataSize != 2 && dataSize != 4 && dataSize != 8))
	{
		return SIZE_MAX;
	}

	const uint8_t* start = instr;
	const uint8_t* end = instr + len;

	int minOffset = 0;
	uint16_t opcode = (instr[0] << 8) | instr[1];
	
	// these are Multiple byte opcodes. 
	if (opcode == 0x0F38 || opcode == 0x0F3A || (opcode >= 0x0F00 && opcode <= 0x0FFF))
	{
		minOffset = 1;
	}

	int offset = 0;
	for (const uint8_t* p = instr; p <= end; p++)
	{
		uint64_t value = 0;
		memcpy(&value, instr + offset, dataSize);

		// Make sure we don't take the opcode as the value
		if (offset <= minOffset)
		{
			offset++;
			continue;
		}

		if (dataSize < 8)
		{
			const uint64_t mask = (1ULL << (dataSize * 8)) - 1;
			value &= mask;
		}

		if (value == stackSize)
		{
			return offset;
		}

		offset++;
	}

	return SIZE_MAX;
}

template<class T>
static inline void PatchValue(uintptr_t address, size_t offset, uint64_t origValue, uint64_t newValue)
{
	T newData = (T)newValue;
	T* addr = (T*)(address + offset);

	assert(*addr == (T)origValue);
	hook::put<T>(addr, newData);
	assert(*addr == newData);
}

struct StackResizes
{
	int oldStackSize;
	int newStackSize;
};

template<int NewStackSize, int kMaxInstructions = 2048, int kMaxStackResizes = 128>
void IncreaseFunctionStack(void* address, std::initializer_list<StackResizes> list)
{
	auto getImmValue = [](const struct ud_operand* operand, uint8_t* size = nullptr) -> int64_t
	{
		if (!operand)
		{
			return 0;
		}

		uint8_t dataSize = 0;
		if (operand->type == UD_OP_IMM)
		{
			dataSize = operand->size / 8; // Immediate size in bytes
		}
		else if (operand->type == UD_OP_MEM && (operand->offset != 0 && operand->offset <= 64))
		{
			dataSize = operand->offset / 8;
		}
		else
		{
			if (size)
			{
				*size = 0;
			}
			return 0;
		}

		int64_t disp = 0;
		switch (dataSize)
		{
			case 1:
				disp = operand->lval.sbyte;
				break;
			case 2:
				disp = operand->lval.sword;
				break;
			case 4:
				disp = operand->lval.sdword;
				break;
			case 8:
				disp = operand->lval.sqword;
				break;
			default:
				__debugbreak();
				break;
		}

		if (size)
		{
			*size = dataSize;
		}

		return disp;
	};

	ud_t ud;
	ud_init(&ud);
	ud_set_mode(&ud, 64);

	// set the program counter
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(address));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(address), INT32_MAX);

	int64_t functionStackSize = -1;
	bool attemptStackRelocation = list.size() >= 1;

	struct StackInfo
	{
	public:
		uintptr_t address;
		size_t offset;
		uint64_t origValue;
		uint8_t dataSize;
	};

	StackInfo stackData[kMaxStackResizes]{};
	size_t stackCount = 0;

	for (int i = 0; i < kMaxInstructions; i++)
	{
		if (!ud_disassemble(&ud))
		{
			// Nothing else to disassemble
			break;
		}
																																																																																																						
		uint8_t size;
		uint64_t dataSize;
		ud_mnemonic_code mnemonic = ud_insn_mnemonic(&ud);

		// if this is a retn, we've likely reached the end of the function.
		if (mnemonic == UD_Iret)
		{
			break;
		}

		auto addr = ud_insn_off(&ud);

        const auto* op0 = ud_insn_opr(&ud, 0);
		const auto* op1 = ud_insn_opr(&ud, 1);

		bool validOperands = op0 && op1;

		// Handle sub/add
		if ((mnemonic == UD_Isub || mnemonic == UD_Iadd) && validOperands 
			&& op0->type == UD_OP_REG && op0->base == UD_R_RSP && op1->type == UD_OP_IMM)
		{
			// op1 is immediate value
			dataSize = getImmValue(op1, &size);

#ifdef VERBOSE
			trace("Found 0x%llx: %s %s, 0x%llx\n",
			ud_insn_off(&ud),
			ud_lookup_mnemonic(mnemonic),
			(op0->base == UD_R_RSP) ? "RSP" : "RBP",
			dataSize);
#endif

			size_t offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);
			stackData[stackCount++] = { addr, offset, dataSize, size };

			// if this is restoring the stack pointer, we can currently assume this is the end of the function and break out here
			if (mnemonic == UD_Iadd)
			{
				break;
			}

			functionStackSize = dataSize;
		}

		// If we haven't gotten the stack size of the function. Then skip.
		if (functionStackSize == -1)
		{
			continue;
		}

		// LEA is a special case as the function may be using it to restore rsp
		if (mnemonic == UD_Ilea && validOperands)
		{
			size_t offset = SIZE_MAX;

			// LEA: op0 = reg, op1 = mem
			if (op0->type == UD_OP_REG && op1->type == UD_OP_MEM && op1->base == UD_R_RSP)
			{
				dataSize = getImmValue(op1, &size);
				offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);

#ifdef VERBOSE
				trace("Found 0x%llx LEA %s, [RSP+0x%llx] (size=%zu, offset=%zu)\n",
				ud_insn_off(&ud),
				ud_lookup_mnemonic(mnemonic),
				dataSize,
				size,
				offset);
#endif

				if (offset == SIZE_MAX)
				{
					continue;
				}

				// Check if this LEA restores RSP
				if (dataSize == functionStackSize)
				{
					stackData[stackCount++] = { addr, offset, dataSize, size };
					break;
				}

				if (attemptStackRelocation)
				{
					stackData[stackCount++] = { addr, offset, dataSize, size };
				}

				continue;
			}
		}

		// if we aren't attempting stack relocation, this code isn't needed.
		if (!attemptStackRelocation || !validOperands)
		{
			continue;
		}

		//op0 - register
		//op1 - immediate

		// if op0 is mem
		if (op0->type == UD_OP_MEM)
		{
			// if immediate value
			if (op1->type == UD_OP_IMM && op0->base == UD_R_RSP)
			{
				dataSize = getImmValue(op0, &size);
#ifdef VERBOSE
				trace("Found 0x%llx: %s [%s%+lld]\n",
				ud_insn_off(&ud),
				ud_lookup_mnemonic(mnemonic),
				(op0->base == UD_R_RSP) ? "RSP" : "RBP",
				dataSize);
#endif
				size_t offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);
				stackData[stackCount++] = { addr, offset, dataSize, size };
				continue;
			}

			if (op1->type == UD_OP_REG && op0->base == UD_R_RSP)
			{
				dataSize = getImmValue(op0, &size);

#ifdef VERBOSE
				trace("Found 0x%llx: %s [%s%+lld]\n",
				ud_insn_off(&ud),
				ud_lookup_mnemonic(mnemonic),
				(op0->base == UD_R_RSP) ? "RSP" : "RBP",
				getImmValue(op0));
#endif
				size_t offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);

				stackData[stackCount++] = { addr, offset, dataSize, size };
				continue;
			}
		}

		if (op0->type == UD_OP_REG && op1->type == UD_OP_MEM && op1->base == UD_R_RSP)
		{
			dataSize = getImmValue(op1, &size);

#ifdef VERBOSE
			trace("Found 0x%llx: %s [%s%+lld]\n",
			ud_insn_off(&ud),
			ud_lookup_mnemonic(mnemonic),
			(op1->base == UD_R_RSP) ? "RSP" : "RBP",
			getImmValue(op1));
#endif
			size_t offset = GetDisplacementOffset(ud, mnemonic, dataSize, size);
			stackData[stackCount++] = { addr, offset, dataSize, size };
			continue;
		}
	}

	int stackFrameReplaced = 0;
	for (const StackInfo& val : stackData)
	{
		int newValue = (val.origValue == functionStackSize) ? NewStackSize : -1;
		if (newValue == -1 && attemptStackRelocation )
		{
			for (auto& value : list)
			{
				if (value.oldStackSize == val.origValue)
				{
					newValue = value.newStackSize;
				}
			}
		}

		if (newValue == -1)
		{
			continue;
		}

#ifdef VERBOSE
		trace("Changing value 0x%x (0x%x): 0x%x -> 0x%x\n", val.address + val.offset, val.address, val.origValue, newValue);
#endif

		int64_t disp = 0;
		switch (val.dataSize)
		{
			// 1 byte / 8 bit
			case 1:
				PatchValue<uint8_t>(val.address, val.offset, val.origValue, newValue);
				break;
			// 2 bytes / 16 bit
			case 2:
				PatchValue<uint16_t>(val.address, val.offset, val.origValue, newValue);
				break;
			// 4 bytes / 32 bit
			case 4:
				PatchValue<uint32_t>(val.address, val.offset, val.origValue, newValue);
				break;
			// 8 bytes / 64 bit
			case 8:
				PatchValue<uint64_t>(val.address, val.offset, val.origValue, newValue);
				break;
			default:
				disp = 0;
				__debugbreak();
				break;
		}

		if (val.origValue == functionStackSize)
		{
			stackFrameReplaced++;
		}
	}

	// Make sure that both the stack frame allocation and stack frame have been replaced.
	assert(stackFrameReplaced == 2);
}

static const uint8_t kMaxPlayers = 128;

namespace rage
{
	using Vec3V = DirectX::XMVECTOR;
}

uint32_t g_playerFocusPositionUpdateBitset[(kMaxPlayers / 32) + 1];
alignas(16) rage::Vec3V g_playerFocusPositions[kMaxPlayers + 1];

extern ICoreGameInit* icgi;
extern CNetGamePlayer* g_players[256];
extern CNetGamePlayer* g_playerListRemote[256];
extern int g_playerListCountRemote;

template<bool Onesync, bool Legacy>
static bool Return()
{
	return icgi->OneSyncEnabled ? Onesync : Legacy;
}

struct PatternPair
{
	std::string_view pattern;
	int offset;
	int operand_remaining = 4;
};

struct PatternClampPair
{
	std::string_view pattern;
	int offset;
	bool clamp;
};

static void RelocateRelative(void* base, std::initializer_list<PatternPair> list, int intendedEntries = -1)
{
	void* oldAddress = nullptr;

	if (intendedEntries >= 0 && list.size() != intendedEntries)
	{
		__debugbreak();
		return;
	}


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
		hook::put<T>(location, entry.newValue);
	}
}

static hook::cdecl_stub<rage::Vec3V*(rage::Vec3V*, CNetGamePlayer*, char*)> _getNetPlayerFocusPosition([]()
{
	return hook::get_pattern("41 22 C2 41 3A C1 0F 93 C0", -0x74);
});

static void (*g_origUpdatePlayerFocusPosition)(void*, CNetGamePlayer*);
static void UpdatePlayerFocusPosition(void* objMgr, CNetGamePlayer* player)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origUpdatePlayerFocusPosition(objMgr, player);
	}

	bool isRemote = player->physicalPlayerIndex() == 31;
	uint8_t playerIndex = player->physicalPlayerIndex();

	// Find the players index if remote.
	if (isRemote)
	{
		for (int i = 0; i < 256; i++)
		{
			if (g_players[i] == player)
			{
				playerIndex = i;
				break;
			}
		}
	}

	if (playerIndex > kMaxPlayers)
	{
		return;
	}

	rage::Vec3V position;
	char outFlag = 0;
	rage::Vec3V* outPosition = _getNetPlayerFocusPosition(&position, player, &outFlag);


	g_playerFocusPositions[playerIndex] = *outPosition;
	if (outFlag != 0)
	{
		g_playerFocusPositionUpdateBitset[playerIndex / 32] |= 1 << (playerIndex % 32);
	}
}

static rage::Vec3V* (*g_origGetNetPlayerRelevancePosition)(rage::Vec3V* position, CNetGamePlayer* player, void* unk);
static rage::Vec3V* GetPlayerFocusPosition(rage::Vec3V* position, CNetGamePlayer* player, uint8_t* unk)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetNetPlayerRelevancePosition(position, player, unk);
	}

	bool isRemote = player->physicalPlayerIndex() == 31;
	uint8_t playerIndex = player->physicalPlayerIndex();

	// Find the players index if remote.
	if (isRemote)
	{
		for (int i = 0; i < 256; i++)
		{
			if (g_players[i] == player)
			{
				playerIndex = i;
				break;
			}
		}
	}

	if (playerIndex > kMaxPlayers)
	{
		return nullptr;
	}

	if (unk)
	{
		uint32_t bitset = g_playerFocusPositionUpdateBitset[playerIndex / 32];
		int bit = playerIndex % 32;
		*unk = (bitset >> bit) & 1;
	}

	*position = g_playerFocusPositions[playerIndex];
	return position;
}

static hook::cdecl_stub<void*(CNetGamePlayer*)> getPlayerPedForNetPlayer([]()
{
	return hook::get_call(hook::get_pattern("48 8B CD 0F 11 06 48 8B D8 E8", -8));
});

static float VectorDistance(rage::Vec3V a, rage::Vec3V b)
{
	rage::Vec3V delta = DirectX::XMVectorSubtract(a, b);
	float dis = DirectX::XMVectorGetX(DirectX::XMVector3Length(delta));
	return dis;
}

static int (*g_origGetPlayersNearPoint)(rage::Vec3V* point, uint32_t unkIndex, void* outIndex, CNetGamePlayer* outArray[32], bool unkVal, float range, bool sorted);
static int GetPlayersNearPoint(rage::Vec3V* point, uint32_t unkIndex, void* outIndex, CNetGamePlayer* outArray[32], bool unkVal, float range, bool sorted)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPlayersNearPoint(point, unkIndex, outIndex, outArray, range, range, sorted);
	}

	CNetGamePlayer* tempArray[kMaxPlayers]{};

	int idx = 0;

	auto playerList = g_playerListRemote;
	for (int i = 0; i < g_playerListCountRemote; i++)
	{
		auto player = playerList[i];

		if (player && getPlayerPedForNetPlayer(player))
		{
			rage::Vec3V vectorPos;

			if (range >= 100000000.0f || VectorDistance(*point, *GetPlayerFocusPosition(&vectorPos, player, nullptr)) < range)
			{
				tempArray[idx] = player;
				idx++;
			}
		}
	}

	if (sorted)
	{
		std::sort(tempArray, tempArray + idx, [point](CNetGamePlayer* a1, CNetGamePlayer* a2)
		{
			rage::Vec3V vectorPos1;
			rage::Vec3V vectorPos2;

			float d1 = VectorDistance(*point, *GetPlayerFocusPosition(&vectorPos1, a1, nullptr));
			float d2 = VectorDistance(*point, *GetPlayerFocusPosition(&vectorPos2, a2, nullptr));

			return (d1 < d2);
		});
	}

	idx = std::min(idx, 32);
	unkIndex = idx;
	std::copy(tempArray, tempArray + idx, outArray);

	return idx;
}

static void (*g_unkRemoteBroadcast)(void*, __int64);
static void unkRemoteBroadcast(void* a1, __int64 a2)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_unkRemoteBroadcast(a1, a2);
	}
}

static uint32_t*(*g_unkPlayerFootstepBitset)(void*, uint32_t*);
static uint32_t* _unkPlayerFootstepBitset(void* self, uint32_t* oldBitset)
{
	// a single uint32_t bitset is passed to this function and is used to set player indexes as flags.
	// This causes issues if we want to go above a playerIndex of 32.
	// In all cases the bitset above is only used for this function. So we can allocate our own bitset here
	// and pass that into the original function restoring behaviour
	uint32_t bitset[(kMaxPlayers / 32) + 1] = {};
	return g_unkPlayerFootstepBitset(self, bitset);
}

static void* (*g_sub_1422B40D4)(void*, uint32_t*, uint32_t*, uint32_t*);
static void* sub_1422B40D4(void* a1, uint32_t* oldBitset, uint32_t* unk, uint32_t* unk2)
{
	// a single uint32_t bitset is passed to this function and is used to set player indexes as flags.
	// This causes issues if we want to go above a playerIndex of 32.
	// In all cases the bitset above is only used for this function. So we can allocate our own bitset here
	// and pass that into the original function restoring behaviour
	uint32_t bitset[(kMaxPlayers / 32) + 1] = {};
	return g_sub_1422B40D4(a1, bitset, unk, unk2);
}

static void* (*g_unkP2PObjectInit)(void*);
static void* unkP2PObjectInit(void* objectMgr)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_unkP2PObjectInit(objectMgr);
	}
	return nullptr;
}

static int* sub_1424(void* a1, int* a2, void* a3, bool a4)
{
	*a2 = 0;
	return a2;
}

static unsigned long (*g_netArrayManager__Update)(void*);
static unsigned long netArrayManager__Update(void* a1)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_netArrayManager__Update(a1);
	}
	return 0;
}

static void* (*g_unkBandwidthTelemetry)(void*, int);
static void* unkBandwidthTelemetry(void* bandwidthMgr, int a2)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_unkBandwidthTelemetry(bandwidthMgr, a2);
	}

	return nullptr;
}

void** g_cachedPlayerArray;

// This function is protected with arxan and is self-healed
static uint32_t GetPlayerFastInstanceState(uint8_t playerIndex)
{
	if (playerIndex < kMaxPlayers + 1)
	{
		hook::FlexStruct* player = reinterpret_cast<hook::FlexStruct*>(g_cachedPlayerArray[playerIndex]);
		if (player)
		{
			return player->At<uint32_t>(0x4);
		}
	}

	return 0;
}

static uint16_t* g_localPlayerInstanceId;
static uint16_t GetPlayerFastInstanceId(uint8_t playerIndex)
{
	if (playerIndex < kMaxPlayers + 1)
	{
		if (playerIndex == rage::GetLocalPlayer()->physicalPlayerIndex())
		{
			return *g_localPlayerInstanceId;
		}

		hook::FlexStruct* player = reinterpret_cast<hook::FlexStruct*>(g_cachedPlayerArray[playerIndex]);
		if (player)
		{
			return player->At<uint16_t>(0x2);
		}
	}
	return 0x100;
}

static HookFunction hookFunction([]()
{
	// Expand Player Damage Array to support more players
	{
		constexpr size_t kDamageArraySize = sizeof(uint32_t) * 256;
		uint32_t* damageArrayReplacement = (uint32_t*)hook::AllocateStubMemory(kDamageArraySize);
		memset(damageArrayReplacement, 0, kDamageArraySize);

		RelocateRelative((void*)damageArrayReplacement, { 
			{ "48 8D 0D ? ? ? ? 44 21 35", 3 },
			{ "4C 8D 25 ? ? ? ? 41 83 3C B4", 3 },
			{ "48 8D 0D ? ? ? ? 85 DB 74", 3 },
			{ "48 8D 15 ? ? ? ? 8B 0C 82", 3 }
		});

		// Patch damage related comparisions
		PatchValue<uint8_t>({ 
			{"3C ? 73 ? 44 21 35", 1, 0x20, kMaxPlayers + 1},
		    {"80 F9 ? 73 ? 0F B6 D1 48 8D 0D", 2, 0x20, kMaxPlayers + 1 },
		    {"80 BB ? ? ? ? ? 73 ? 40 0F B6 C7", 6, 0x20, kMaxPlayers + 1 },
			//CWeaponDamageEvent::HandleReply
			{"40 80 FF ? 73 ? 40 38 3D", 3, 0x20, kMaxPlayers + 1},
			//CWeaponDamageEvent::_checkIfDead
			{ "80 3D ? ? ? ? ? 40 8A 68", 6, 0x20, kMaxPlayers + 1 },
		});
	}

	// Expand Player Cache data
	{
		static size_t kCachedPlayerSize = sizeof(void*) * (kMaxPlayers + 1);
		g_cachedPlayerArray = (void**)hook::AllocateStubMemory(kCachedPlayerSize);
		memset(g_cachedPlayerArray, 0, kCachedPlayerSize);

		g_localPlayerInstanceId = hook::get_address<uint16_t*>(hook::get_pattern("66 89 05 ? ? ? ? B2", 2));

		RelocateRelative((void*)g_cachedPlayerArray, {
			{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 66 83 79", 3 },
			{ "48 8D 0D ? ? ? ? 48 8B F2 BD", 3 },
			{ "BD ? ? ? ? 8B C3 48 8D 0D", 10 },
			{ "0F B6 C3 48 8D 0D ? ? ? ? 48 8B 0C C1", 6 },
			{ "48 8D 0D ? ? ? ? 48 8B 0C ? 48 85 C9 74 ? 8B 41", 3},
			{ "48 8D 0D ? ? ? ? 48 8B 04 C1 8A 40", 3 },
			{ "4C 8D 0D ? ? ? ? 48 63 05 ? ? ? ? 48 8B C8", 3 },
			{ "48 8D 0D ? ? ? ? 88 15 ? ? ? ? 88 15 ? ? ? ? 88 15", 3 },
			{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 83 79", 3 },
			{ "48 8D 15 ? ? ? ? 48 8B 14 CA 48 85 D2 74 ? 39 72", 3 },
			{ "48 8D 0D ? ? ? ? 48 8B C3 48 8B 0C D9", 3 },
			{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 8A 41 ? EB", 3 },
			{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 40 88 79", 3 },
			{ "48 8D 15 ? ? ? ? 88 19", 3 },
			{ "48 8D 3D ? ? ? ? 48 8B 0C DF", 3 },
			{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 66 89 ?", 3 },
			{ "48 8D 1D ? ? ? ? 48 8B 1C C3 48 85 DB 74 ? 66 39 73", 3 },
			{ "48 8D 0D ? ? ? ? 48 8B 04 C1 40 88 78", 3 },
			{ "48 8D 0D ? ? ? ? 48 8B 0C C1 48 85 C9 74 ? 66 3B 59", 3 },
			{ "48 8D 15 ? ? ? ? 84 C9 75", 3 }
		}, 20);

		PatchValue<uint8_t>({
			// player cached getters
			{ "E8 ? ? ? ? 84 C0 74 ? 40 88 3D ? ? ? ? EB", 23, 0x20, kMaxPlayers + 1 },
			//{ "80 F9 ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? E8", 2, 0x20, kMaxPlayers + 1},
			//{ "83 F9 ? 7C ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8D 0D ? ? ? ? 48 8B 0C D9", 2, 0x20, kMaxPlayers + 1},
			{ "80 FA ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 8A CB", 2, 0x20, kMaxPlayers + 1},
			{ "80 7B ? ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 0F B6 43 ? 48 8D 0D", 3, 0x20, kMaxPlayers + 1},
			{ "80 F9 ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 0F B6 DB", 2, 0x20, kMaxPlayers + 1}, // removeCachedPlayerEntry
			{ "80 F9 ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 0F B6 C3", 2, 0x20, kMaxPlayers + 1}, // _setPlayerCachedFastInstanceId
			{ "80 FB ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 8A CB", 2, 0x20, kMaxPlayers + 1},
			{ "48 85 C0 74 ? 83 61 ? ? B8 ? ? ? ? 66 83 61", -57, 0x20, kMaxPlayers + 1 }, // addCachedPlayerEntry
			//{ "83 F9 ? 7C ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8D 0D ? ? ? ? 48 8B C3", 2, 0x20, kMaxPlayers + 1 }, // getPlayerCachedInTutorialState
			{ "83 F8 ? 72 ? C3 CC 0F 48 8B", 2, 0x20, kMaxPlayers + 1 },
			{ "80 FB ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 75 ? B0", 2, 0x20, kMaxPlayers + 1},
			{ "83 F9 ? 7C ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8D 0D ? ? ? ? 48 8B C3", 2, 0x20, kMaxPlayers + 1 }
		});

		// Handle GetPlayerFastInstanceState getters
		// This function is axran
		{
			std::initializer_list<PatternPair> pairs = {
				{ "E8 ? ? ? ? 83 F8 ? 75 ? B8 ? ? ? ? EB ? 8A CB", 0 },
				{ "E8 ? ? ? ? 83 F8 ? 75 ? 48 85 DB 0F 84", 0 },
				{ "E8 ? ? ? ? 83 F8 ? 74 ? 32 C0 EB ? B0 ? 48 83 C4 ? 5B C3 CC 79", 0 },
				{ "E8 ? ? ? ? 0F B6 4F ? 3B C1", 0 },
				{ "E8 ? ? ? ? 83 F8 ? 74 ? 40 8A CE", 0 }
			};

			for (auto& entry : pairs)
			{
				hook::call(hook::get_pattern(entry.pattern, entry.offset), GetPlayerFastInstanceState);
			}
		}

		// Handle GetPlayerFastInstanceId getters
		// This function is also axran
		{
			std::initializer_list<PatternPair> pairs = {
				{ "E8 ? ? ? ? 98 48 83 C4 ? 5B", 0 },
				{ "E8 ? ? ? ? B9 ? ? ? ? 66 3B C1 0F 85", 0 },
				{ "E8 ? ? ? ? 0F B7 0D ? ? ? ? 66 3B C1", 0 },
				{ "E8 ? ? ? ? 66 41 3B C4 75 ? 83 BF", 0 },
				{ "E8 ? ? ? ? 0F BF C8 41 3B CC", 0 },
				{ "E8 ? ? ? ? 66 3B C3 74 ? 49 8B CE", 0 }, 
				{ "E8 ? ? ? ? 0F BF F8 41 BF", 0 }, 
				{ "E8 ? ? ? ? 40 84 ED 74 ? 80 BC 24", 0 }
			};

			for (auto& entry : pairs)
			{
				hook::call(hook::get_pattern(entry.pattern, entry.offset), GetPlayerFastInstanceId);
			}
		}
	}

	// Replace 32-sized unknown CGameArray related array
	{
		void** unkPlayerArray = (void**)hook::AllocateStubMemory(sizeof(void*) * kMaxPlayers + 1);

		RelocateRelative((void*)unkPlayerArray, { 
			{ "48 8D 3D ? ? ? ? 48 8B 3C C7 48 85 FF 75", 3},
			{ "48 8D 1D ? ? ? ? 48 8B 33 48 85 F6 74 ? 48 8B 06", 3 },
			{ "48 8D 3D ? ? ? ? BD ? ? ? ? 48 8D 35", 3 }
		}, 3);
	}

	// Replace 33-sized player bandwidth related array.
	{
		constexpr size_t kBandwithArraySize = sizeof(unsigned int) * kMaxPlayers + 2;
		void** bandwidthRelatedArray = (void**)hook::AllocateStubMemory(kBandwithArraySize);
		memset(bandwidthRelatedArray, 0, kBandwithArraySize);

		RelocateRelative((void*)bandwidthRelatedArray, {
			{ "48 8D 3D ? ? ? ? B9 ? ? ? ? F3 AB 48 8D 8B", 3 },
			{ "48 8D 15 ? ? ? ? C7 04 82", 3 },
			{ "48 8D 0D ? ? ? ? 89 1C 81", 3 },
			{ "48 8D 05 ? ? ? ? 8B 14 B8 3B EA", 3 },
			{ "48 8D 0D ? ? ? ? 8B 04 81 C3 90 66 40 53", 3 }
		});

		PatchValue<uint32_t>({
			{ "B9 ? ? ? ? F3 AB 48 8D 8B ? ? ? ? 33 D2", 1, 0x20, kMaxPlayers },
		});
	}

	// Patch CNetworkDamageTracker
	{
		// 32/31 comparsions
		PatchValue<uint8_t>({
			{"80 7A ? ? 0F 28 F2 48 8B F2", 3, 0x20, kMaxPlayers + 1},
			{"80 7A ? ? 48 8B F9 72", 3, 0x20, kMaxPlayers + 1}
		});
	}

	// Patch netObject to account for >32 players
	{
		// 32/31 32bit comparsions
		PatchValue<uint32_t>({ 
			// rage::netObject::DependencyThreadUpdate
			{ "41 BF ? ? ? ? 8A 8C 10", 2, 0x20, kMaxPlayers + 1}
		});

		PatchValue<uint8_t>({
			// rage::netObject::CanCreateWithNoGameObject
			{ "80 79 ? ? 73 ? 32 C0", 3, 0x20, kMaxPlayers + 1 },
			// rage::netObject::CanPassControl
			{ "3C ? 73 ? 3A 46", 1, 0x20, kMaxPlayers + 1},
			// rage::netObject::SetOwner
			{ "80 F9 ? 73 ? E8 ? ? ? ? 48 8B D8 EB", 2, 0x20, kMaxPlayers },
			// rage::netObject::IsPendingOwnerChange
			{ "80 79 ? ? 0F 92 C0 C3 48 8B 91", 3, 0x20,  kMaxPlayers + 1 },
			// rage::netObject::StartSynchronising
			{ "8B E9 4C 8B 33", 48, 0x20, kMaxPlayers + 1 },

			// NetObjVehicle scene/viewport related.
			{ "49 8B 7E ? 48 85 FF 0F 84 ? ? ? ? 48 8B 2D", 23, 0x20, kMaxPlayers + 1 }
		});
	}

	// rage::netPlayerMgrBase
	{
		 // Change count from 32 to 128
		PatchValue<uint32_t>({ 
			{ "C7 83 ? ? ? ? ? ? ? ? BA ? ? ? ? 48 89 AB", 6, 0x20, kMaxPlayers }
		});
	}

	// Replace 32/31 comparisions
	{
		std::initializer_list<PatternClampPair> list = {
			//CNetGamePlayer::IsPhysical
			{ "80 79 ? ? 0F 92 C0 C3 48 89 5C 24", 3, false },
			//rage::netPlayer::IsPhysical
			{ "80 7B ? ? 73 ? B2", 3, false },

			// unk local player related
			{ "80 7D ? ? 48 8B F8 72 ? 32 C0", 3, false },
			
			// CPhysical::_CorrectSyncedPosition
			{ "40 80 FE ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84", 3, false },
			// getNetPlayerFromGamerHandleIfInSession
			{ "48 8B C8 48 8B D6 E8 ? ? ? ? 84 C0 75 ? FE C3", 19, false },
			
			// Related to CNetworkPopulationResetMgr
			//{ "40 80 FF ? 73 ? 48 8B 4E", 3, false},
			//{ "80 FB ? 72 ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 8B 7C 24", 2, false},
			//{ "80 FB ? 72 ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 83 C4 ? 41 5F 41 5E 41 5D 41 5C 5F C3 83 FA", 2, false },

			// Ped Combat related
			{ "80 FA ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 0F B6 C3 BA", 2, false },
			{ "80 FA ? 0F 83 ? ? ? ? 48 8B 05", 2, false },
			{ "80 3B ? 73 ? 48 8B CE", 2, false },

			// CNetObjProximityMigrateable::_getRelevancePlayers
			//{ "40 80 FF ? 0F 82 ? ? ? ? 0F 28 74 24 ? 4C 8D 5C 24 ? 49 8B 5B ? 48 8B C6", 3, false },

			//CNetObjGame::CanClone
			{ "80 7A ? ? 49 8B F8 48 8B DA 48 8B F1 72", 3, false},

			// getNetworkEntityOwner
			{ "80 F9 ? 72 ? 33 C0 C3 E9 ? ? ? ? 48 89 5C 24", 2, false },

			//FindNetworkPlayerPed
			{ "83 F9 ? 73 ? E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8", 2, false },

			//rage::netObjectIDMgr::TryToAllocateInitialObjectIDs, removes need for ScMultiplayerImpl size patches
			//Also removes the need to patch session entering logic.
			{ "80 79 ? ? 0F 83 ? ? ? ? 44 38 7B", 3, false },
			{ "80 FB ? 72 ? BA ? ? ? ? C7 44 24 ? ? ? ? ? 41 B9 ? ? ? ? 48 8D 0D ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 8A CB", 2, false },

			// rage::netObject::_doesPlayerHaveControlOverObject
			{ "80 79 ? ? 72 ? B0", 3, false },

			//{ "80 7F ? ? 72 ? 41 B9 ? ? ? ? C7 44 24", 3, false },

			// Native Fixes
			{ "83 FB ? 73 ? 45 33 C0", 2, false }, // 0x862C5040F4888741
			{ "83 F9 ? 0F 83 ? ? ? ? B2", 2, false }, // 0x236321F1178A5446
			{ "83 F9 ? 73 ? 80 3D", 2, false }, // 0x93DC1BE4E1ABE9D1
			{ "48 85 C9 74 0F 83 FE 20", 7, false }, // 0x66B57B72E0836A76

			// netObject vtable functions
			{ "49 81 ? C0 7A 02 00 E8 F3 ? ? 00 40 8A F0 ? 20", 16, false },
			
			//TODO: Investigate further if these patches are needed
			//{ "E8 76 68 E6 FD 84 C0 0F 84 96 00 00 00 ? ? ? 20", 16, false },
			//{ "48 8B CB E8 77 66 05 00 84 C0 74 41 40 80 FF 20", 15, false }, 

			// player iteration
			{ "80 FB ? 72 ? B0 ? 48 8B 5C 24 ? 48 83 C4", 2, false },

			// getPlayer
			{ "83 F9 ? 73 ? E8 ? ? ? ? 48 83 C4 ? C3 90 23 E0", 2, false },
			{ "80 F9 ? 73 ? E8 ? ? ? ? 48 8B D8 48 85 C0", 2, false }
		};

		for (auto& entry : list)
		{
			auto location = hook::pattern(entry.pattern).count(1).get(0).get<uint8_t>(entry.offset);
			auto origVal= *location;
			assert(origVal == (entry.clamp ? 31 : 32));
			hook::put<uint8_t>(location, (entry.clamp ? kMaxPlayers : kMaxPlayers + 1));
		}
	}

	// Replace 32 array iterations
	{
		std::initializer_list<PatternClampPair> list = {
			// Player Cache Data Initalization
			{ "44 8D 41 ? 33 D2 4C 8D 0D", 3, true },
		};

		for (auto& entry : list)
		{
			auto location = hook::pattern(entry.pattern).count(1).get(0).get<uint8_t>(entry.offset);
			auto origVal = *location;
			assert(origVal == 32 || origVal == 31);
			hook::put<uint8_t>(location, origVal == 31 ? kMaxPlayers - 1 : kMaxPlayers);
		}
	}

	// Support entity migration for >32 in CNetObjProximityMigrateable::_passOutOfScope & CNetObjPedBase::_passOutOfScope
	{
		// 256 * 8: 256 players, ptr size
		// 256 * 4: 256 players, int size
		constexpr int ptrsBase = 0x20;
		constexpr int stackSize = (ptrsBase + (256 * 8) + (256 * 4));
		constexpr int intsBase = ptrsBase + (256 * 8);

		// CNetObjProximityMigrateable::_passOutOfScope
		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 48 8B D9 0F 84", -0x15), { { 0x120, intsBase } });
		// CNetObjPedBase::_passOutOfScope
		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 48 8B 71 ? 48 8B D9 48 85 F6", -0x15), { { 0x120, intsBase } });
	}

	// Resize stack to support >32 players for boat population turn taking
	{
		constexpr int ptrsBase = 0x30;
		constexpr int stackSize = ptrsBase + (128 * 8) + 0x10;
		constexpr int intBase = ptrsBase + (128 * 8);

		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 8B E9 E8", -0x10), { { 0x120, intBase } });
	}

	// Resize stack to support >32 players when updating task sequences
	{
		constexpr int ptrsBase = 0x40;
		constexpr int stackSize = ptrsBase + (kMaxPlayers * 8) + 0x10;
		constexpr int intBase = ptrsBase + (kMaxPlayers * 8);

		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 81 EC ? ? ? ? 41 8A D9 45 8B F8", -24), { { 0x188, intBase } });
	}

	// Resize stack to support >32 players with REQUEST_IS_VOLUME_EMPTY netEvent
	{
		constexpr int ptrsBase = 0x20;
		constexpr int arraySize = (kMaxPlayers * 8);
		constexpr int stackSize = ptrsBase + arraySize;

		IncreaseFunctionStack<stackSize, 2048, 8>(hook::get_pattern<char>("48 81 EC ? ? ? ? 41 8A D8 4C 8B F2 4C 8B F9", -0x14), { });
		// memset 0x100 -> arraySize
		hook::put<uint32_t>(hook::get_pattern<uint32_t>("41 B8 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 84 DB", 2), arraySize);
	}

#if 0
	// Resize stack for CTheScripts::_getClosestPlayer
	{
		// 0x50: previous stack size
		// 16: extra int[3] ontop of present int[2] allocation and stack alignment 
		constexpr int stackSize = 0x50 + 16;
		IncreaseFunctionStack<stackSize>(hook::get_pattern<char>("48 83 EC ? 0F 29 70 ? 33 ED 0F 29 78 ? 0F 57 FF", -0x10), {});
	}
#endif


	// Patch bubble join to prevent writing out of bounds for player objects
	{
		auto location = hook::get_pattern("44 0F B6 4E ? 0F B6 40");

		static struct : jitasm::Frontend
		{
			uintptr_t retnSuccess;
			uintptr_t retnFail;

			void Init(uintptr_t success, uintptr_t failure)
			{
				retnSuccess = success;
				retnFail = failure;
			}

			virtual void InternalMain() override
			{
				// Original code
				movzx(r9d, byte_ptr[rsi + 0x10]);
				movzx(eax, byte_ptr[rax + 0x20]);

				cmp(eax, 0x20);
				jge("Fail");

				mov(rcx, retnSuccess);
				jmp(rcx);

				L("Fail");
				mov(rcx, retnFail);
				jmp(rcx);
			}
		} patchStub;

		const uintptr_t retnSuccess = (uintptr_t)location + 9;
		const uintptr_t retnFail = retnSuccess + 0x19;

		hook::nop(location, 9);
		patchStub.Init(retnSuccess, retnFail);
		hook::jump_rcx(location, patchStub.GetCode());
	}

	// Extend bitshift in order to not lead to crashes
	hook::put<uint8_t>(hook::get_pattern("48 C1 EA ? 8B 44 94 ? 0F AB C8 48 8B CE", 3), 8);

	// Skip unused host kick related >32-unsafe arrays in onesync
	hook::call(hook::get_pattern("E8 ? ? ? ? 84 C0 75 ? 8B 05 ? ? ? ? 33 C9 89 44 24"), Return<true, false>);
	hook::call(hook::get_pattern("E8 ? ? ? ? 84 C0 75 ? 80 7B ? ? 73 ? 48 8B CB"), Return<true, false>);

	// nop "Last Too Many Objects ACK" network log, indexes an 32 sized array without a < 32 check
	{
		auto location = hook::get_pattern("4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 4C 8B 10 45 8B 8C 8E");
		hook::nop(location, 32);
	}

	// Remove network text chat. Unused in RedM/RDR but uses 32 sized arrays with minimal checks
	{
		// Update
		hook::nop(hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? 45 84 F6 74", -23), 28);

		// AddPlayer
		hook::nop(hook::get_pattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8B 0D ? ? ? ? 44 8B C7"), 34);
	}

	// Rewrite functions to account for extended players
	MH_Initialize();
	// Don't broadcast script info for script created vehicles in OneSync.
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8B EC 48 81 EC ? ? ? ? 48 83 79"), unkRemoteBroadcast, (void**)&g_unkRemoteBroadcast);
	// Don't call init related object iteration (32-sized player object array with no >32 check)
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D")), unkP2PObjectInit, (void**)&g_unkP2PObjectInit);
	MH_CreateHook(hook::get_pattern("48 8B 0F 0F B6 51 19 48 03 D2 49 8B 5C D6 08", -49), unkP2PObjectInit, NULL);

	// Update Player Focus Positions to support 128 players.
	MH_CreateHook(hook::get_pattern("0F A3 D0 0F 92 C0 88 06", -0x76), GetPlayerFocusPosition, (void**)&g_origGetNetPlayerRelevancePosition);
	MH_CreateHook(hook::get_pattern("74 ? 4C 8D 44 24 ? C6 44 24 ? ? 48 8B D6", -68), UpdatePlayerFocusPosition, (void**)&g_origUpdatePlayerFocusPosition);

	// Allocate greater sized bitsets to avoid stack corruption
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 4C 89 44 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 65 4C 8B 14 25"), sub_1422B40D4, (void**)&g_sub_1422B40D4);
	MH_CreateHook(hook::get_pattern("4D 8B 04 C0 4E 39 3C 01 75 ? 33 C0 89 02", -0x39), _unkPlayerFootstepBitset, (void**)&g_unkPlayerFootstepBitset);

	MH_CreateHook(hook::get_pattern("33 DB 0F 29 70 D8 49 8B F9 4D 8B F0", -0x1B), GetPlayersNearPoint, (void**)&g_origGetPlayersNearPoint);

	//TEMP: Potentially can overflow and lead to issues, and this logic isn't important in onesync at the moment.
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 65 48 8B 0C 25 ? ? ? ? 4C 8B F2"), sub_1424, NULL);
	hook::return_function(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 54 41 56 41 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 8B F0"));
	MH_CreateHook(hook::get_pattern("48 89 4C 24 ? 53 55 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8B F9"), netArrayManager__Update, (void**)&g_netArrayManager__Update);
	MH_CreateHook(hook::get_pattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 F6"), unkBandwidthTelemetry, (void**)&g_unkBandwidthTelemetry);

	MH_EnableHook(MH_ALL_HOOKS);
});
