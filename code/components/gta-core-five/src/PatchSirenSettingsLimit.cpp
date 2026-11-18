#include <StdInc.h>
#include <Hooking.h>
#include <jitasm.h>
#include <atArray.h>
#include <RageParser.h>

#include <shared_mutex>
#include <unordered_map>

//
// carcols.meta siren setting IDs and carvariations.meta `sirenSettings` references are parsed as 8-bit integers, and
// the game maps them to an index in the global siren settings list using a 256-entry lookup table. Any ID above 255 is
// truncated and collides with another siren setting, and no more than 256 unique siren settings can be referenced.
// This patch parses the IDs as 16-bit values, resolves them by searching the list, and stores a 16-bit index in the
// model info. IDs 255 and 65535 remain reserved as "unset".
//
// It also raises the number of lights per siren setting from 20 to 32 (bones siren21..siren32). CSirenSettings grows
// to hold 32 lights; the vehicle siren update handles 20 lights by bone id, so it runs a second time for lights 21..32
// with a shadow siren setting, a second siren state, and a bone table that maps siren1..siren12 to siren21..siren32.
//

static constexpr uint32_t kVanillaSirenCount = 20;
static constexpr uint32_t kSirenCount = 32;

struct CSirenLight
{
	struct Pattern
	{
		float delta;
		float start;
		float speed;
		uint32_t sequencer;
		uint8_t multiples;
		bool direction;
		bool syncToBpm;
		uint8_t pad;
	};

	Pattern rotation;
	Pattern flashiness;
	uint8_t data[0x20];
};

static_assert(sizeof(CSirenLight) == 0x48);

struct CSirenSettings
{
	uint32_t id;
	uint8_t pad[0x20];
	uint32_t headTailSequencers[4];
	uint8_t headTailMultiples[4];
	uint8_t pad2[4];
	CSirenLight sirens[kSirenCount];
	uint32_t numSirens;
	const char* name;
};

static_assert(offsetof(CSirenSettings, sirens) == 0x3C);
static_assert(sizeof(CSirenSettings) == 0x948);

static constexpr uint32_t kSirensEnd = offsetof(CSirenSettings, numSirens) - offsetof(CSirenSettings, sirens);

static constexpr size_t kVariationSirenIndex = 0x49;
static constexpr size_t kVariationSirenId = 0x4B;
static constexpr size_t kModelSirenIndexLow = 0x53B;
static constexpr size_t kModelSirenIndexHigh = 0x55F;

static char** g_carcols;

static atArray<CSirenSettings>& GetSirenSettingsList()
{
	return *reinterpret_cast<atArray<CSirenSettings>*>(*g_carcols + 0x48);
}

static bool IsUnsetSirenId(uint16_t id)
{
	return id == 0xFF || id == 0xFFFF;
}

static uint16_t FindSirenSettingsIndex(uint16_t id)
{
	auto& sirens = GetSirenSettingsList();

	for (uint16_t i = 0; i < sirens.GetCount(); i++)
	{
		if (static_cast<uint16_t>(sirens[i].id) == id)
		{
			return i;
		}
	}

	return 0;
}

static void ResolveVariationSirenIndex(char* variation)
{
	auto& id = *reinterpret_cast<uint16_t*>(variation + kVariationSirenId);
	auto& index = *reinterpret_cast<uint8_t*>(variation + kVariationSirenIndex);
	auto& sirens = GetSirenSettingsList();

	if (IsUnsetSirenId(id))
	{
		if (index < sirens.GetCount())
		{
			id = static_cast<uint16_t>(sirens[index].id);
		}

		return;
	}

	index = static_cast<uint8_t>(std::min<uint16_t>(FindSirenSettingsIndex(id), 0xFF));
}

static uint8_t StoreModelSirenIndex(char* modelInfo, char* variation)
{
	auto id = *reinterpret_cast<uint16_t*>(variation + kVariationSirenId);
	uint16_t index = IsUnsetSirenId(id) ? *reinterpret_cast<uint8_t*>(variation + kVariationSirenIndex) : FindSirenSettingsIndex(id);

	*reinterpret_cast<uint8_t*>(modelInfo + kModelSirenIndexLow) = index & 0xFF;
	*reinterpret_cast<uint8_t*>(modelInfo + kModelSirenIndexHigh) = index >> 8;

	return index & 0xFF;
}

// Bone ids of siren1..siren20 / siren_glass1..siren_glass20 in the vehicle hierarchy.
static constexpr uint32_t kSirenBone = 0x89;
static constexpr uint32_t kSirenGlassBone = kSirenBone + kVanillaSirenCount;
static constexpr uint32_t kSirenBoneEnd = kSirenGlassBone + kVanillaSirenCount;

// Second update pass for lights 21..32: the update only reads the model info at +0xB0 (bone table) and through
// GetModelSirenSettings, so it is given a stand-in model info.
struct SirenPass
{
	CSirenSettings settings;
	uint8_t* boneTable;
	uint8_t boneTableData[0x10 + kSirenBoneEnd];
	alignas(8) char model[0xB8];
};

static thread_local SirenPass* t_sirenPass;

static CSirenSettings* GetModelSirenSettings(char* modelInfo)
{
	if (t_sirenPass && modelInfo == t_sirenPass->model)
	{
		return &t_sirenPass->settings;
	}

	uint16_t index = *reinterpret_cast<uint8_t*>(modelInfo + kModelSirenIndexLow) | (*reinterpret_cast<uint8_t*>(modelInfo + kModelSirenIndexHigh) << 8);
	auto& sirens = GetSirenSettingsList();

	if (index == 0 || index >= sirens.GetCount())
	{
		return nullptr;
	}

	return &sirens[index];
}

// rage::crSkeleton -> crSkeletonData: bones at +0x20 (0x50 bytes each, name at +0x38), bone count at +0x5E.
static void FindSirenBones(char* skeleton, int (&sirenBones)[kSirenCount + 1], int (&glassBones)[kSirenCount + 1])
{
	std::fill(std::begin(sirenBones), std::end(sirenBones), -1);
	std::fill(std::begin(glassBones), std::end(glassBones), -1);

	auto skeletonData = *reinterpret_cast<char**>(skeleton);
	auto bones = *reinterpret_cast<char**>(skeletonData + 0x20);
	auto boneCount = *reinterpret_cast<uint16_t*>(skeletonData + 0x5E);

	for (int i = 0; i < boneCount; i++)
	{
		auto name = *reinterpret_cast<const char**>(bones + i * 0x50 + 0x38);

		if (!name || _strnicmp(name, "siren", 5) != 0)
		{
			continue;
		}

		bool glass = _strnicmp(name + 5, "_glass", 6) == 0;
		int number = atoi(name + (glass ? 11 : 5));

		if (number >= 1 && number <= kSirenCount)
		{
			(glass ? glassBones : sirenBones)[number] = i;
		}
	}
}

static constexpr uint32_t kExtraSirenCount = kSirenCount - kVanillaSirenCount;

// Bone indices of siren21..siren32 and siren_glass21..siren_glass32 by the model's bone id table, for the damage loops.
// `vanilla` holds the table's siren1..20 / siren_glass1..20 entries, so a table freed and reused by another model misses.
struct ExtraSirenBones
{
	int8_t vanilla[kVanillaSirenCount * 2];
	int8_t sirens[kExtraSirenCount];
	int8_t glass[kExtraSirenCount];
};

static std::shared_mutex g_extraSirenBonesMutex;
static std::unordered_map<const int8_t*, ExtraSirenBones> g_extraSirenBones;

static const ExtraSirenBones* FindExtraSirenBones(const int8_t* table)
{
	auto it = g_extraSirenBones.find(table);

	if (it == g_extraSirenBones.end() || memcmp(it->second.vanilla, table + 0x10 + kSirenBone, sizeof(it->second.vanilla)) != 0)
	{
		return nullptr;
	}

	return &it->second;
}

static int32_t GetExtraSirenBone(const int8_t* table, uint64_t slot, bool glass)
{
	std::shared_lock lock(g_extraSirenBonesMutex);
	auto bones = FindExtraSirenBones(table);

	if (!bones || slot >= kExtraSirenCount)
	{
		return -1;
	}

	return (glass ? bones->glass : bones->sirens)[slot];
}

static bool BuildSirenPass(SirenPass& pass, char* modelInfo, char* skeleton, const CSirenSettings& settings)
{
	int sirenBones[kSirenCount + 1];
	int glassBones[kSirenCount + 1];
	FindSirenBones(skeleton, sirenBones, glassBones);

	// The game's own table must agree with the names, or the bone layout is not what this expects.
	auto gameTable = **reinterpret_cast<int8_t***>(modelInfo + 0xB0);
	if (gameTable[0x10 + kSirenBone] >= 0 && gameTable[0x10 + kSirenBone] != sirenBones[1])
	{
		return false;
	}

	bool known;
	{
		std::shared_lock lock(g_extraSirenBonesMutex);
		known = FindExtraSirenBones(gameTable) != nullptr;
	}

	if (!known)
	{
		ExtraSirenBones bones;
		memcpy(bones.vanilla, gameTable + 0x10 + kSirenBone, sizeof(bones.vanilla));

		for (uint32_t i = 0; i < kExtraSirenCount; i++)
		{
			auto siren = sirenBones[kVanillaSirenCount + 1 + i];
			auto glass = glassBones[kVanillaSirenCount + 1 + i];
			bones.sirens[i] = siren < 0x80 ? siren : -1;
			bones.glass[i] = glass < 0x80 ? glass : -1;
		}

		std::unique_lock lock(g_extraSirenBonesMutex);
		g_extraSirenBones.insert_or_assign(gameTable, bones);
	}

	pass.settings = settings;
	memmove(&pass.settings.sirens[0], &settings.sirens[kVanillaSirenCount], sizeof(CSirenLight) * (settings.numSirens - kVanillaSirenCount));
	pass.settings.numSirens = settings.numSirens - kVanillaSirenCount;
	memset(pass.settings.headTailSequencers, 0, sizeof(pass.settings.headTailSequencers));
	memset(pass.settings.headTailMultiples, 0, sizeof(pass.settings.headTailMultiples));

	// The update offsets each light's rotation and flash by its index (i or 20 - i); keep the phase of lights 21..32.
	for (uint32_t i = 0; i < pass.settings.numSirens; i++)
	{
		for (auto pattern : { &pass.settings.sirens[i].rotation, &pass.settings.sirens[i].flashiness })
		{
			pattern->start += (pattern->direction ? -1.0f : 1.0f) * kVanillaSirenCount * pattern->delta;
		}
	}

	memset(pass.boneTableData, 0xFF, sizeof(pass.boneTableData));
	for (uint32_t i = 0; i < pass.settings.numSirens; i++)
	{
		auto siren = sirenBones[kVanillaSirenCount + 1 + i];
		auto glass = glassBones[kVanillaSirenCount + 1 + i];
		pass.boneTableData[0x10 + kSirenBone + i] = siren < 0x80 ? siren : 0xFF;
		pass.boneTableData[0x10 + kSirenGlassBone + i] = glass < 0x80 ? glass : 0xFF;
	}

	pass.boneTable = pass.boneTableData;
	*reinterpret_cast<uint8_t***>(pass.model + 0xB0) = &pass.boneTable;
	return true;
}

// Register arguments of the siren update; everything from the fifth argument on is on the stack.
struct SirenUpdateRegisters
{
	uint64_t rdx;
	uint64_t r8;
	uint64_t r9;
	uint64_t xmm1;
	uint64_t xmm2;
	uint64_t xmm3;
};

static constexpr int kSirenUpdateStackArgs = 8;

static void (*g_callSirenUpdate)(char* vehicle, const SirenUpdateRegisters* registers, const uint64_t* stackArgs);
static int g_sirenUpdateModelArg;
static uint32_t g_vehicleSirenStateOffset;
static uint32_t g_sirenStateSize;
static uint32_t g_vehicleBrokenSirensOffset;

static void UpdateVehicleSirens(char* vehicle, const SirenUpdateRegisters* registers, const uint64_t* stackArgs)
{
	g_callSirenUpdate(vehicle, registers, stackArgs);

	auto modelInfo = reinterpret_cast<char*>(stackArgs[g_sirenUpdateModelArg - 4]);
	auto skeleton = reinterpret_cast<char*>(stackArgs[g_sirenUpdateModelArg - 3]);
	auto settings = modelInfo ? GetModelSirenSettings(modelInfo) : nullptr;
	auto& state = *reinterpret_cast<char**>(vehicle + g_vehicleSirenStateOffset);

	if (!settings || settings->numSirens <= kVanillaSirenCount || settings->numSirens > kSirenCount || !skeleton || !state)
	{
		return;
	}

	static thread_local SirenPass pass;
	if (!BuildSirenPass(pass, modelInfo, skeleton, *settings))
	{
		return;
	}

	uint64_t args[kSirenUpdateStackArgs];
	memcpy(args, stackArgs, sizeof(args));
	args[g_sirenUpdateModelArg - 4] = reinterpret_cast<uint64_t>(pass.model);

	// One bit per light; the game only uses bits 0..19, so lights 21..32 keep theirs in bits 20..31.
	auto& brokenSirens = *reinterpret_cast<uint32_t*>(vehicle + g_vehicleBrokenSirensOffset);
	auto vehicleBrokenSirens = brokenSirens;
	auto vehicleState = state;
	brokenSirens = vehicleBrokenSirens >> kVanillaSirenCount;
	state = vehicleState + g_sirenStateSize;
	t_sirenPass = &pass;

	g_callSirenUpdate(vehicle, registers, args);

	t_sirenPass = nullptr;
	state = vehicleState;
	brokenSirens = vehicleBrokenSirens;
}

static void (*g_constructSirenState)(char* state, void* model);

// Each vehicle siren state is allocated twice as large; the second half holds the state of lights 21..32.
static void ConstructSirenStates(char* state, void* model)
{
	g_constructSirenState(state, model);
	g_constructSirenState(state + g_sirenStateSize, model);
}

// hook::call reaches its target through "mov rax, target; jmp rax". Stubs placed in the middle of game code must keep
// rax, so they are copied next to the executable and called directly.
static void CallStub(void* site, jitasm::Frontend& stub)
{
	auto code = stub.GetCode();
	auto size = stub.GetCodeSize();
	auto copy = static_cast<char*>(hook::AllocateStubMemory(size));
	memcpy(copy, code, size);

	hook::put<uint8_t>(site, 0xE8);
	hook::put<int32_t>(static_cast<char*>(site) + 1, static_cast<int32_t>(copy - static_cast<char*>(site) - 5));
}

// Replaces a bone id table read in a siren damage loop, "movsx r32, byte [table + id + disp]" or
// "cmp byte [table + id + disp], 0xFF": ids past the vanilla range read lights 21..32 from GetExtraSirenBone.
struct SirenBoneReadStub : jitasm::Frontend
{
	const uint8_t* site;
	uint32_t limit;
	bool glass;

	SirenBoneReadStub(const uint8_t* site, uint32_t limit, bool glass)
		: site(site), limit(limit), glass(glass)
	{
	}

	void InternalMain() override
	{
		const Reg64 regs[] = { rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi };
		const Reg32 regs32[] = { eax, ecx, edx, ebx, esp, ebp, esi, edi };
		const XmmReg xmms[] = { xmm0, xmm1, xmm2, xmm3, xmm4, xmm5 };

		// "0F BE modrm sib disp8" or "80 7C sib disp8 FF".
		bool compare = site[0] == 0x80;
		auto sib = compare ? site[2] : site[3];
		auto dest = (site[2] >> 3) & 7;
		auto& base = regs[sib & 7];
		auto& index = regs[(sib >> 3) & 7];
		auto disp = compare ? site[3] : site[4];

		// Pushed registers below, by register number: rax, rcx, rdx.
		const int slots[] = { 0x30, 0x28, 0x20 };
		assert(compare || dest < 3);

		cmp(index, static_cast<int32_t>(limit));
		jae("extra");

		if (compare)
		{
			cmp(byte_ptr[base + index + disp], 0xFF);
		}
		else
		{
			movsx(regs32[dest], byte_ptr[base + index + disp]);
		}

		ret();

		L("extra");
		push(rax);
		push(rcx);
		push(rdx);
		push(r8);
		push(r9);
		push(r10);
		push(r11);
		sub(rsp, 0x80);

		for (int i = 0; i < 6; i++)
		{
			movups(xmmword_ptr[rsp + 0x20 + i * 0x10], xmms[i]);
		}

		mov(r8, base);
		mov(r9, index);
		mov(rcx, r8);
		lea(rdx, qword_ptr[r9 - static_cast<int32_t>(limit)]);
		mov(r8d, glass ? 1 : 0);
		mov(rax, reinterpret_cast<uintptr_t>(&GetExtraSirenBone));
		call(rax);

		for (int i = 0; i < 6; i++)
		{
			movups(xmms[i], xmmword_ptr[rsp + 0x20 + i * 0x10]);
		}

		lea(rsp, qword_ptr[rsp + 0x80]);

		if (compare)
		{
			cmp(al, 0xFF);
		}
		else
		{
			movsx(eax, al);
			mov(qword_ptr[rsp + slots[dest]], rax);
		}

		pop(r11);
		pop(r10);
		pop(r9);
		pop(r8);
		pop(rdx);
		pop(rcx);
		pop(rax);
		ret();
	}
};

// The siren damage loops (impact, collision, smash all, hiding broken light bones) walk bone ids siren1..siren20 and
// keep one broken bit per light; run them over 32 lights and patch their bone id table reads.
static void PatchSirenDamageLoop(uint8_t* top, uint8_t* end)
{
	std::vector<uint8_t*> sites;

	for (auto p = top; p + 5 <= end; p++)
	{
		if ((p[-1] & 0xF0) == 0x40)
		{
			continue;
		}

		bool read = p[0] == 0x0F && p[1] == 0xBE && (p[2] & 0xC7) == 0x44 && (p[4] == 0x10 || p[4] == 0x24);
		bool compare = p[0] == 0x80 && p[1] == 0x7C && p[3] == 0x10 && p[4] == 0xFF;

		if (read || compare)
		{
			sites.push_back(p);
		}
	}

	assert(sites.size() == 1 || sites.size() == 2);

	for (size_t i = 0; i < sites.size(); i++)
	{
		// The second read is the light's glass bone: by the siren id at +0x24 (glass = siren + 20), or by the glass id.
		bool glass = i == 1;
		uint32_t limit = (glass && sites[i][4] == 0x10) ? kSirenGlassBone + kVanillaSirenCount : kSirenGlassBone;

		auto stub = new SirenBoneReadStub(sites[i], limit, glass);
		CallStub(sites[i], *stub);
	}
}

struct VariationInitStub : jitasm::Frontend
{
	jitasm::Reg64 variation;

	explicit VariationInitStub(const jitasm::Reg64& variation)
		: variation(variation)
	{
	}

	void InternalMain() override
	{
		mov(dword_ptr[variation + 0x48], static_cast<int32_t>(0xFFFF0000));
		mov(byte_ptr[variation + 0x4C], 0);
		ret();
	}
};

static HookFunction hookFunction([]()
{
	// Widen the parsed members: CSirenSettings::id to u32 (followed by a float at +4), CVehicleModelInfoVariation::sirenSettings to u16 (tail padding at +0x4C).
	{
		auto location = hook::get_pattern<char>("48 83 3D ? ? ? ? 00 0F 85 ? ? ? ? B9 E8 05 00 00");
		auto descriptor = hook::get_address<char*>(location, 3, 8) - 0x10;
		auto members = *reinterpret_cast<rage::parMemberDefinition***>(descriptor + 0x18);

		for (auto member = members; *member; member++)
		{
			if ((*member)->hash == HashString("id"))
			{
				(*member)->type = rage::parMemberType::UInt32;
			}
			else if ((*member)->hash == HashString("sirens"))
			{
				(*member)->arrayElemCount = kSirenCount;
			}
			else if ((*member)->hash == HashString("name"))
			{
				(*member)->offset = offsetof(CSirenSettings, name);
			}
		}

		hook::put<uint32_t>(location + 15, sizeof(CSirenSettings));

		hook::get_pattern<rage::parMemberDefinition>("0D 9B EB 84 00 00 00 00 ? ? ? ? ? ? ? ? 02")->type = rage::parMemberType::UInt16;

		// CVehicleModelInfoVarGlobal::Sirens
		hook::get_pattern<rage::parMemberDefinition>("F5 14 F8 9A 00 00 00 00 ? ? ? ? ? ? ? ? 0D 00")->arrayElemSize = sizeof(CSirenSettings);
	}

	// CSirenSettings storage for 32 lights: element stride, light array length, and the light count/name that follow it.
	{
		for (auto [pattern, offset] : { std::pair{ "48 69 C9 E8 05 00 00 E8 ? ? ? ? 33 FF", 3 }, { "48 81 C6 E8 05 00 00 49 2B EF", 3 },
				 { "48 69 C9 E8 05 00 00 48 03 D1", 3 }, { "48 69 C0 E8 05 00 00 48 03 03", 3 }, { "B9 E8 05 00 00 E8 ? ? ? ? 48 8B C8", 1 } })
		{
			hook::put<uint32_t>(hook::get_pattern(pattern, offset), sizeof(CSirenSettings));
		}

		// atArray<CSirenSettings> allocation helper, only present in newer builds.
		for (auto [pattern, offset] : { std::pair{ "48 69 C9 E8 05 00 00 E8 ? ? ? ? 48 8B F0", 3 }, { "48 81 C7 E8 05 00 00 48 FF CD 75", 3 } })
		{
			if (auto match = hook::pattern(pattern).count_hint(1); match.size() == 1)
			{
				hook::put<uint32_t>(match.get_first(offset), sizeof(CSirenSettings));
			}
		}

		hook::put<uint32_t>(hook::get_pattern("BA 13 00 00 00 48 8D 41 1C", 1), kSirenCount - 1);
		hook::put<uint32_t>(hook::get_pattern("44 89 81 A0 05 00 00 48 8B C1", 3), kSirensEnd);
		hook::put<uint32_t>(hook::get_pattern("48 63 82 A0 05 00 00 4C 8B C9", 3), kSirensEnd);
		hook::put<uint32_t>(hook::get_pattern("89 81 A0 05 00 00 4E 8D 04 C2", 2), kSirensEnd);
		hook::put<uint32_t>(hook::get_pattern("C7 83 DC 05 00 00 14 00 00 00", 2), offsetof(CSirenSettings, numSirens));

		auto name = hook::get_pattern<char>("48 8B 87 E0 05 00 00 48 89 83 E0 05 00 00");
		hook::put<uint32_t>(name + 3, offsetof(CSirenSettings, name));
		hook::put<uint32_t>(name + 10, offsetof(CSirenSettings, name));
	}

	// CSirenSettings constructor: initialize the whole ID instead of its low byte.
	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				mov(dword_ptr[rbx], 0xFF);
				mov(dword_ptr[rbx + 4], 0x3F800000);
				ret();
			}
		} stub;

		auto location = hook::get_pattern("C6 03 FF C7 43 04 00 00 80 3F");
		hook::nop(location, 10);
		CallStub(location, stub);
	}

	// CVehicleModelInfoVariation constructors: zero the high byte of `sirenSettings`.
	{
		static VariationInitStub stubRcx(jitasm::Frontend::rcx);
		static VariationInitStub stubRax(jitasm::Frontend::rax);

		auto locationRcx = hook::get_pattern("C7 41 48 00 00 FF FF C3");
		hook::nop(locationRcx, 7);
		CallStub(locationRcx, stubRcx);

		auto locationRax = hook::get_pattern("C7 40 48 00 00 FF FF EB");
		hook::nop(locationRax, 7);
		CallStub(locationRax, stubRax);
	}

	// CSirenSettings copy: copy the whole ID ("mov eax, [rdx]" / "mov [rcx], eax").
	{
		auto location = hook::get_pattern<uint8_t>("8A 02 48 8B FA 48 8B D9 88 01 8B 42 04");
		hook::put<uint8_t>(location, 0x8B);
		hook::put<uint8_t>(location + 8, 0x89);
	}

	// Merging siren settings from a new carcols file: compare whole IDs when skipping duplicates.
	{
		auto location = hook::get_pattern<uint8_t>("48 69 ED E8 05 00 00 48 03 2E 44 0F B6 4D 00");
		constexpr uint8_t payload[] = { 0x44, 0x8B, 0x4D, 0x00, 0x90 }; // "mov r9d, [rbp]" and "nop".
		memcpy(location + 10, payload, sizeof(payload)); // replace "movzx r9d, byte ptr [rbp]".
		hook::put<uint32_t>(location + 3, sizeof(CSirenSettings));

		auto compare = hook::get_pattern<uint8_t>("44 38 08 74 ? 48 FF C1 48 05 E8 05 00 00");
		hook::put<uint8_t>(compare + 1, 0x39); // "cmp [rax], r9d".
		hook::put<uint32_t>(compare + 10, sizeof(CSirenSettings));
	}

	// The 8-bit ID -> index table is no longer used, and reassigning IDs through it would clobber 16-bit IDs.
	hook::return_function(hook::get_pattern("48 8B 6E 48 8B FB 48 69 FF E8 05 00 00 80 3C 2F FF", -0x2D));

	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				mov(rcx, rdx);
				mov(rax, reinterpret_cast<uintptr_t>(&ResolveVariationSirenIndex));
				jmp(rax);
			}
		} stub;

		hook::jump(hook::get_pattern("44 38 42 4B 75 ? 0F B6 4A 49"), stub.GetCode());
	}

	auto getSirenSettings =hook::get_pattern<char>("8A 81 3B 05 00 00 84 C0 74 ? 48 8B 0D");
	g_carcols = hook::get_address<char**>(getSirenSettings + 13);
	hook::jump(getSirenSettings, GetModelSirenSettings);

	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				mov(rcx, rdi);
				mov(rdx, rsi);
				mov(rsi, qword_ptr[r11 + 0x18]);
				mov(rsp, r11);
				pop(rdi);
				mov(rax, reinterpret_cast<uintptr_t>(&StoreModelSirenIndex));
				jmp(rax);
			}
		} stub;

		hook::jump(hook::get_pattern("8A 46 49 49 8B 73 18 88 87 3B 05 00 00 49 8B E3"), stub.GetCode());
	}

	// Vehicle siren flag: a model has sirens if either byte of its index is set.
	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				push(rax);
				mov(al, byte_ptr[r14 + kModelSirenIndexHigh]);
				test(al, al);
				jnz("compare");
				mov(al, byte_ptr[r14 + kModelSirenIndexLow]);
				L("compare");
				cmp(al, r12b);
				pop(rax);
				ret();
			}
		} stub;

		auto location = hook::get_pattern("45 38 A6 3B 05 00 00 48 8B 8E");
		hook::nop(location, 7);
		CallStub(location, stub);
	}

	// Vehicle siren state: allocate a second state for lights 21..32 next to the first one.
	{
		auto allocation = hook::get_pattern<char>("B9 88 01 00 00 E8 ? ? ? ? 48 8B D8");
		g_sirenStateSize = *reinterpret_cast<uint32_t*>(allocation + 1);
		hook::put<uint32_t>(allocation + 1, g_sirenStateSize * 2);

		auto construct = hook::get_pattern<char>("E8 ? ? ? ? EB 03 49 8B DC 48 8B 06 48 8B CE 48 89 9E");
		g_vehicleSirenStateOffset = *reinterpret_cast<uint32_t*>(construct + 19);
		hook::set_call(&g_constructSirenState, construct);
		hook::call(construct, ConstructSirenStates);
	}

	// Vehicle siren update: run a second pass for lights 21..32.
	{
		char* call = nullptr;

		auto calls = hook::pattern("E8 ? ? ? ? 49 8B CE E8");
		for (size_t i = 0; i < calls.size(); i++)
		{
			auto candidate = calls.get(i).get<char>();
			if (hook::get_call(candidate + 8) == getSirenSettings)
			{
				call = candidate;
				break;
			}
		}

		assert(call);
		auto update = hook::get_call(call);

		// Model info argument index: the update reads it from [rbp + x] with rbp = entry rsp + "lea rbp, [rax + frame]".
		{
			auto modelLoad = hook::get_pattern<char>("48 8B 85 ? ? 00 00 48 8B 80 B0 00 00 00 48 8B 08 ? 0F BE ? ? 10");
			auto prologue = std::search(update, update + 0x40, "\x48\x8D\xA8", "\x48\x8D\xA8" + 3);
			assert(prologue != update + 0x40);

			auto frame = *reinterpret_cast<int32_t*>(prologue + 3);
			g_sirenUpdateModelArg = (*reinterpret_cast<int32_t*>(modelLoad + 3) + frame - 8) / 8;

			// Broken light check right before it: "mov eax, [r15 + rax*4 + offset]" / "bt eax, edx".
			auto broken = std::search(modelLoad - 0x40, modelLoad, "\x8B\x84\x87", "\x8B\x84\x87" + 3);
			assert(broken != modelLoad);
			g_vehicleBrokenSirensOffset = *reinterpret_cast<uint32_t*>(broken + 3);
		}

		static struct : jitasm::Frontend
		{
			uintptr_t update;

			void InternalMain() override
			{
				push(rbx);
				sub(rsp, 0x20 + kSirenUpdateStackArgs * 8);
				mov(rbx, rdx);

				for (int i = 0; i < kSirenUpdateStackArgs; i++)
				{
					mov(rax, qword_ptr[r8 + i * 8]);
					mov(qword_ptr[rsp + 0x20 + i * 8], rax);
				}

				mov(rdx, qword_ptr[rbx + offsetof(SirenUpdateRegisters, rdx)]);
				mov(r8, qword_ptr[rbx + offsetof(SirenUpdateRegisters, r8)]);
				mov(r9, qword_ptr[rbx + offsetof(SirenUpdateRegisters, r9)]);
				movq(xmm1, qword_ptr[rbx + offsetof(SirenUpdateRegisters, xmm1)]);
				movq(xmm2, qword_ptr[rbx + offsetof(SirenUpdateRegisters, xmm2)]);
				movq(xmm3, qword_ptr[rbx + offsetof(SirenUpdateRegisters, xmm3)]);
				mov(rax, update);
				call(rax);

				add(rsp, 0x20 + kSirenUpdateStackArgs * 8);
				pop(rbx);
				ret();
			}
		} replay;

		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				sub(rsp, 0x58);
				mov(qword_ptr[rsp + 0x20 + offsetof(SirenUpdateRegisters, rdx)], rdx);
				mov(qword_ptr[rsp + 0x20 + offsetof(SirenUpdateRegisters, r8)], r8);
				mov(qword_ptr[rsp + 0x20 + offsetof(SirenUpdateRegisters, r9)], r9);
				movq(qword_ptr[rsp + 0x20 + offsetof(SirenUpdateRegisters, xmm1)], xmm1);
				movq(qword_ptr[rsp + 0x20 + offsetof(SirenUpdateRegisters, xmm2)], xmm2);
				movq(qword_ptr[rsp + 0x20 + offsetof(SirenUpdateRegisters, xmm3)], xmm3);
				lea(rdx, qword_ptr[rsp + 0x20]);
				lea(r8, qword_ptr[rsp + 0x58 + 0x28]);
				mov(rax, reinterpret_cast<uintptr_t>(&UpdateVehicleSirens));
				call(rax);
				add(rsp, 0x58);
				ret();
			}
		} entry;

		replay.update = reinterpret_cast<uintptr_t>(update);
		g_callSirenUpdate = reinterpret_cast<decltype(g_callSirenUpdate)>(replay.GetCode());
		hook::call(call, entry.GetCode());
	}

	// Siren damage loops: "cmp reg64, 0x9D" / "jl" back to a body that reads the model's bone id table (at +0xB0).
	{
		auto readsBoneTable = [](uint8_t* top, uint8_t* end)
		{
			for (auto p = top; p + 7 <= end; p++)
			{
				if (p[0] == 0x48 && p[1] == 0x8B && p[2] >= 0x80 && p[2] <= 0x97 && *reinterpret_cast<uint32_t*>(p + 3) == 0xB0)
				{
					return true;
				}
			}

			return false;
		};

		auto patchLoop = [&](uint8_t* bound, int immOffset)
		{
			auto branch = bound + immOffset + 4;
			auto top = (branch[0] == 0x7C) ? branch + 2 + *reinterpret_cast<int8_t*>(branch + 1)
				: (branch[0] == 0x0F && branch[1] == 0x8C) ? branch + 6 + *reinterpret_cast<int32_t*>(branch + 2) : nullptr;

			if (!top || top >= bound || bound - top > 0x300 || !readsBoneTable(top, bound))
			{
				return;
			}

			PatchSirenDamageLoop(top, bound);
			hook::put<uint32_t>(bound + immOffset, kSirenGlassBone + kExtraSirenCount);
		};

		auto cmpRegImm = hook::pattern("? 81 ? 9D 00 00 00");
		for (size_t i = 0; i < cmpRegImm.size(); i++)
		{
			auto bound = cmpRegImm.get(i).get<uint8_t>();
			if ((bound[0] == 0x48 || bound[0] == 0x49) && bound[2] >= 0xF8)
			{
				patchLoop(bound, 3);
			}
		}

		auto cmpRaxImm = hook::pattern("48 3D 9D 00 00 00");
		for (size_t i = 0; i < cmpRaxImm.size(); i++)
		{
			patchLoop(cmpRaxImm.get(i).get<uint8_t>(), 2);
		}

		// Newer builds keep the impact loop's bound in r12, which also seeds the glass id in ebx:
		// "mov r12d, 0x9D; xor r15d, r15d; mov r13d, n; mov ebx, r12d" -> "mov ebx, 0x9D; lea r12d, [rbx + 12]; ...".
		auto regBound = hook::pattern("41 BC 9D 00 00 00 45 33 FF 41 BD ? ? 00 00 41 8B DC");
		if (regBound.size() == 1)
		{
			auto location = regBound.get_first<uint8_t>();
			auto top = location + 18;
			auto end = std::search(top, top + 0x200, "\x49\x3B\xC4\x0F\x8C", "\x49\x3B\xC4\x0F\x8C" + 5);

			if (end != top + 0x200)
			{
				PatchSirenDamageLoop(top, end);

				uint8_t code[18] = { 0xBB, 0x9D, 0x00, 0x00, 0x00, 0x44, 0x8D, 0x63, kExtraSirenCount, 0x45, 0x33, 0xFF };
				memcpy(code + 12, location + 9, 6);
				memcpy(location, code, sizeof(code));
			}
		}
	}
});
