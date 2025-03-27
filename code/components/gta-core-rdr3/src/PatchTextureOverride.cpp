#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>
#include <scrEngine.h>
#include <MinHook.h>
#include "Hooking.Stubs.h"

struct UnkTextureLayerModData
{
	char unk[12];
};

struct textureOverlay
{
	uintptr_t m_unkType; // 0
	uint32_t m_albedoHash; // 8
	uint32_t m_normalHash; // 12
	uint32_t m_materialHash; // 16
	uint32_t m_modTexture; // 20
	uint32_t m_paletteHash; // 24
	UnkTextureLayerModData m_layerMods[4]; // 28 - 75 (4 * 12 bytes each, 48 bytes total)
	uint8_t m_colorType; // 76
	char pad[3]; // 77-79
	float m_roughness; // 80
	float m_alpha; // 84
	float m_modAlpha; // 88
	uint8_t m_hasModLayers; // 92
	uint8_t m_tint1; // 93
	uint8_t m_tint2; // 94
	uint8_t m_tint3; // 95
	uint8_t m_albedoTextureIndex; // 96
	uint8_t m_sheetGridIndex; // 97
	char pad2[6]; // 98-103
};

// UAVOT - Unordered Access View Of Texture
struct textureOverride
{
	textureOverlay m_textureOverlays[16]; // 0
	uintptr_t m_overlaysCount; // 1664
	uintptr_t m_albedoTexBlendTexturePtr; // 1672
	uintptr_t m_normalTexBlendTexturePtr; // 1680
	uintptr_t m_materialTexBlendTexturePtr; // 1688
	uintptr_t m_albedoTexBlendTextureIntermediatePtr; // 1696
	uintptr_t m_normalTexBlendTextureIntermediatePtr; // 1704
	uintptr_t m_materialTexBlendTextureIntermediatePtr; // 1712
	uintptr_t m_albedoUAVOT[6]; // 1720
	uintptr_t m_albedoUAVOTSize; // 1768
	uintptr_t m_normalUAVOT[6]; // 1776
	uintptr_t m_normalUAVOTSize; // 1824
	uintptr_t m_materialUAVOT[6]; // 1832
	uintptr_t m_materialUAVOTSize; // 1880
	uint16_t m_textureOverrideReuseCount; // 1888
	uint16_t unk_143EC3622; // 1890
	uint8_t m_normalUnkValue; // 1892
	uint8_t m_materialUnkValue; // 1893
	uint8_t m_UnkValue; // 1894
	uint8_t m_UnkValue2; // 1895
	uint8_t m_textureOverrideOwnerId; // 1896
	char m_flags[7]; // 1897
};

struct PatternPair
{
	std::string_view pattern;
	int offset;
	intptr_t address;
};

static void RelocateAbsolute(std::initializer_list<PatternPair> list)
{
	for (const auto& entry : list)
	{
		auto location = hook::get_pattern<int32_t>(entry.pattern, entry.offset);
		hook::put<int32_t>(location, entry.address - hook::get_adjusted(0x140000000));
	}
}

static void RelocateRelative(std::initializer_list<PatternPair> list, intptr_t newPtr)
{
	void* oldAddress = nullptr;
	for (const auto& entry : list)
	{
		auto location = hook::pattern(entry.pattern).count(1).get(0).get<int32_t>(entry.offset);
		if (!oldAddress)
		{
			oldAddress = hook::get_address<void*>(location);
		}
		assert(hook::get_address<void*>(location) == oldAddress);
		hook::put<int32_t>(location, newPtr - (intptr_t)location - 4);
	}
}

textureOverride* textureOverrideArray;
static uint32_t textureOverrideArraySize = 512;


static hook::cdecl_stub<bool(uint32_t)> doesTextureOverrideExists([]()
{
	return hook::get_pattern("0F B7 C1 3B 05 ? ? ? ? 7D ? 0F B7 C1 4C 8D 05 ? ? ? ? 48 69 D0");
});

static HookFunction hookFunction([]()
{
	textureOverrideArray = reinterpret_cast<textureOverride*>(hook::AllocateStubMemory(sizeof(textureOverride) * textureOverrideArraySize));

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_TEXTURE", [](fx::ScriptContext& context)
	{
		 uint32_t fullTextureId = context.GetArgument<int>(0);

		 if (doesTextureOverrideExists(fullTextureId))
		 {
			 uint16_t textureId = fullTextureId & 0xFFFF;
			 textureOverrideArray[textureId].unk_143EC3622 = 0;
		 }
	});

    fx::ScriptEngine::RegisterNativeHandler("DOES_TEXTURE_EXIST", [](fx::ScriptContext& context)
	{
		 uint32_t fullTextureId = context.GetArgument<int>(0);
         bool exist = doesTextureOverrideExists(fullTextureId); 
         context.SetResult<bool>(exist);
	});


	RelocateAbsolute({
	// Albedo
	{ "48 8D 86 ? ? ? ? FF C1", 3, (intptr_t)&textureOverrideArray[0].m_albedoTexBlendTextureIntermediatePtr },
	{ "4C 8D 8E ? ? ? ? C6 44 24 ? ? 48 8D 8E", 3, (intptr_t)&textureOverrideArray[0].m_albedoTexBlendTexturePtr },
	{ "48 8D 8E ? ? ? ? 48 03 CB 48 03 C3", 3, (intptr_t)&textureOverrideArray[0].m_albedoUAVOT },
	// older build
	// C6 84 33 ? ? ? ? ? E9 3
	// 48 8D 8E ? ? ? ? C6 44 24 ? ? 48 8D 86 ? ? ? ? E9 3 i 15
	// 4C 8D 8E ? ? ? ? E9 3
	// Normal
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8 ? ? ? ? 55", -42, (intptr_t)&textureOverrideArray[0].m_normalUnkValue },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8 ? ? ? ? 55", -29, (intptr_t)&textureOverrideArray[0].m_normalUAVOT },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8 ? ? ? ? 55", -17, (intptr_t)&textureOverrideArray[0].m_normalTexBlendTextureIntermediatePtr },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8 ? ? ? ? 55", 3, (intptr_t)&textureOverrideArray[0].m_normalTexBlendTexturePtr },

	// older build
	// 48 8D 8E ? ? ? ? C6 44 24 ? ? 48 8D 86 ? ? ? ? 48 03 CB 3 i  15
	// C6 84 33 ? ? ? ? ? 48 89 54 24 3
	// 4C 8D 8E ? ? ? ? 48 03 C3 3
	// Material
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8 ? ? ? ? 8B D5", -42, (intptr_t)&textureOverrideArray[0].m_materialUnkValue },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8 ? ? ? ? 8B D5", -29, (intptr_t)&textureOverrideArray[0].m_materialUAVOT },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8 ? ? ? ? 8B D5", -17, (intptr_t)&textureOverrideArray[0].m_materialTexBlendTextureIntermediatePtr },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8 ? ? ? ? 8B D5", 3, (intptr_t)&textureOverrideArray[0].m_materialTexBlendTexturePtr },

	// Other
	{ "C6 84 33 ? ? ? ? ? 48 83 EF", 3, (intptr_t)&textureOverrideArray[0].m_UnkValue },
	});

	std::initializer_list<PatternPair> overlayPatchList = {
		{ "48 8D 2D ? ? ? ? 48 69 F8 ? ? ? ? 8A 84 2F", 3 },
		{ "E9 ? ? ? ? CC 48 89 5C 24 ? 57 48 83 EC ? BF ? ? ? ? 48 8D 1D ? ? ? ? 48 8D 8B", -53 },
		{ "48 8D 15 ? ? ? ? 48 69 C8 ? ? ? ? 8A 84 11 ? ? ? ? A8 ? 75 ? 8B 9C 11", 3 },
		{ "E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 8B 06 89 87", 42 },
		{ "48 8D 0D ? ? ? ? 84 C0 75 ? 8A 84 0B", 3 },
		{ "48 8D 1D ? ? ? ? 8A 83", 3 },
		{ "4C 8D 05 ? ? ? ? 48 69 D0 ? ? ? ? C1 E9", 3 },
		{ "48 8D 35 ? ? ? ? 48 69 D8 ? ? ? ? 8A 84 33", 3 },
		{ "4C 8D 25 ? ? ? ? 48 69 D8", 3 },
		{ "4C 8D 25 ? ? ? ? 8B E9", 3 },
		{ "3B 9A ? ? ? ? 7D ? 48 6B CB ? F3 0F 11 74 11 ? 48 8B 5C 24 ? 0F 28 74 24 ? 48 83 C4 ? 5F C3 48 8B C4", -29 },
		{ "48 6B CB ? F3 0F 11 74 11 ? 48 8B 5C 24 ? 0F 28 74 24 ? 48 83 C4 ? 5F C3 48 89 5C 24", -37 },
		{ "48 8D 15 ? ? ? ? 48 69 C8 ? ? ? ? 8A 84 11 ? ? ? ? A8 ? 75 ? 48 03 D1 85 DB 78 ? 3B 9A ? ? ? ? 7D ? 48 6B CB ? 40 88 74 11", 3 },
		{ "48 8D 05 ? ? ? ? 41 8A 84 06", 3 },
		{ "48 8D 2D ? ? ? ? 48 69 D8", 3 },
		{ "48 8D 0D ? ? ? ? 8B 55 ? 48 83 C1", 3 },
		{ "48 8D 0D ? ? ? ? 44 8B 05 ? ? ? ? 48 83 C1 ? 48 03 CE 41 B9 ? ? ? ? 8B D7", 3 },
		{ "48 8D 0D ? ? ? ? 44 8B 05 ? ? ? ? 48 83 C1 ? 48 03 CE 41 B9 ? ? ? ? 8B D3", 3 },
		{ "48 8D 1D ? ? ? ? 89 44 1E", 3 },
		{ "48 8D 15 ? ? ? ? 48 69 C8 ? ? ? ? 8A 84 11 ? ? ? ? A8 ? 75 ? 48 03 D1 85 DB 78 ? 3B 9A ? ? ? ? 7D ? 8A 44 24", 3 },
		{ "48 8D 05 ? ? ? ? 41 39 8C 03", 3 },
		{ "48 8D 05 ? ? ? ? 41 0F B7 84 03 ? ? ? ? C1 E0 ? 41 0B C2 89 03 E9 ? ? ? ? 48 8D 05", 3 },
		{ "48 8D 05 ? ? ? ? 41 0F B7 84 03 ? ? ? ? C1 E0 ? 41 0B C2 89 03 E9 ? ? ? ? 90", 3 },
		{ "48 8D 15 ? ? ? ? 48 69 C8 ? ? ? ? 8A 84 11 ? ? ? ? A8 ? 75 ? 8B 06", 3 },
		{ "4C 8D 05 ? ? ? ? 83 CE", 3 }, // fix older builds
		{ "4C 8D 05 ? ? ? ? 89 33", 3 }, // fix older builds
		{ "4C 8D 05 ? ? ? ? FF C7 3B FE 0F 8C ? ? ? ? 48 83 C4", 3 }, // fix older builds
		{ "48 8D 05 ? ? ? ? 48 03 F0 4C 8D 83", -23 }, // fix older builds
	};

	RelocateRelative(overlayPatchList, (intptr_t)&textureOverrideArray->m_textureOverlays);

	// Helper lambda to patch a pointer given a pattern
	auto patchPointer = [&](const char* pattern, int offset, auto memberPtr)
	{
		auto loc = hook::get_pattern<int32_t>(pattern, offset);
		hook::put<int32_t>(loc, (intptr_t)memberPtr - (intptr_t)loc - 4);
	};

	patchPointer("48 8D 05 ? ? ? ? 3B 1C 01", 3, &textureOverrideArray->m_overlaysCount);
	patchPointer("48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 8B 04 08 48 83 C4 ? 5B C3 4C 8D 0D", 3, &textureOverrideArray->m_albedoTexBlendTexturePtr);
	patchPointer("48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 8B 04 08 48 83 C4 ? 5B C3 48 63 15", 3, &textureOverrideArray->m_normalTexBlendTexturePtr);
	patchPointer("48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 8B 04 08 48 83 C4 ? 5B C3 F3 0F 10 05", 3, &textureOverrideArray->m_materialTexBlendTexturePtr);
	patchPointer("48 8D 1D ? ? ? ? 45 33 FF 44 8D 71", 3, &textureOverrideArray->m_normalUAVOTSize);
	patchPointer("48 8D 05 ? ? ? ? 41 8B C8 48 69 D1", 3, &textureOverrideArray->m_textureOverrideReuseCount);
	patchPointer("48 8D 05 ? ? ? ? 66 FF 04 01", 3, &textureOverrideArray->unk_143EC3622);
	patchPointer("48 8D 05 ? ? ? ? 66 FF 0C 01", 3, &textureOverrideArray->unk_143EC3622);
	patchPointer("48 8D 05 ? ? ? ? 88 1C 01", 3, &textureOverrideArray->m_textureOverrideOwnerId);
	patchPointer("48 8D 05 ? ? ? ? 38 08", 3, &textureOverrideArray->m_textureOverrideOwnerId);

	std::initializer_list<PatternPair> flagsPatterns = {
		{ "48 8D 15 ? ? ? ? 8A 02", 3 },
		{ "75 ? 42 8A 04 01 A8 ? 74 ? 42 8A 04 01 A8 ? 74 ? 42 8A 04 01", -17 },
		{ "42 8A 04 01 A8 ? 75 ? 42 8A 04 01 A8 ? 74 ? 42 8A 04 01 A8 ? 74 ? B2", -11 },
		{ "48 8D 15 ? ? ? ? 48 69 C8 ? ? ? ? 8A 04 11", 3 },
	};

	RelocateRelative(flagsPatterns, (intptr_t)&textureOverrideArray->m_flags);


	static struct : jitasm::Frontend
	{
		intptr_t failLocation;
		intptr_t textureCountLocation;
		void Init(intptr_t location, intptr_t textureCountLocation)
		{
			this->failLocation = location + 43;
			this->textureCountLocation = textureCountLocation;
		}

		void InternalMain() override
		{
			cmp(eax, textureOverrideArraySize);
			jge("fail");

			add(eax, r10d);
			mov(rax, textureCountLocation);
			mov(dword_ptr[rax], eax);
			dec(eax);
			ret();

			L("fail");
			mov(rax, failLocation);
			jmp(rax);
		}
	} allocationStub;

	auto textureCountLocation = hook::get_address<intptr_t>(hook::get_pattern("48 63 05 ? ? ? ? 45 33 C0", 3));
	auto allocatePedTextureOverrideLocation = hook::get_pattern("83 F8 ? 7D ? 41 03 C2");
	hook::nop(allocatePedTextureOverrideLocation, 17);

	allocationStub.Init(reinterpret_cast<intptr_t>(allocatePedTextureOverrideLocation), textureCountLocation);
	hook::jump(allocatePedTextureOverrideLocation, allocationStub.GetCode());

	 //auto allocatePedTextureOverrideLocation = hook::get_pattern<char>("83 F8 ? 7D ? 41 03 C2", 2);
	 //hook::put<uint8_t>(allocatePedTextureOverrideLocation, textureOverrideArraySize);

	static struct : jitasm::Frontend
	{
		intptr_t jumpBack;
		void Init(intptr_t location)
		{
			this->jumpBack = location + 7;
		}

		void InternalMain() override
		{
			xor(r15d, r15d);
			lea(r14d, qword_ptr[rcx + textureOverrideArraySize - 1]);
			mov(rax, jumpBack);
			jmp(rax);
		}
	} unkStub;

	auto unknownNormalProcessLocation = hook::get_pattern("45 33 FF 44 8D 71");
	hook::nop(unknownNormalProcessLocation, 7);

	unkStub.Init(reinterpret_cast<intptr_t>(unknownNormalProcessLocation));
	hook::jump(unknownNormalProcessLocation, unkStub.GetCode());

	// auto unknownNormalProcessLocation = hook::get_pattern<char>("44 8D 71 ? 48 8B 4B", 3);
	// hook::put<uint8_t>(unknownNormalProcessLocation, textureOverrideArraySize - 1);

	auto clearPedOverlaysTableLocation = hook::get_pattern<char>("E9 ? ? ? ? CC 48 89 5C 24 ? 57 48 83 EC ? BF ? ? ? ? 48 8D 1D ? ? ? ? 48 8D 8B", -48);
	hook::put<uint32_t>(clearPedOverlaysTableLocation, textureOverrideArraySize);

	auto unkLocation = hook::get_pattern<char>("48 83 C4 ? 5F C3 CC CC C2 ? ? CC C2 ? ? CC C2 ? ? CC 48 83 EC", -53);
	hook::put<uint32_t>(unkLocation, textureOverrideArraySize);

	auto unkLocation2 = hook::get_pattern<char>("BF ? ? ? ? BD ? ? ? ? E9 ? ? ? ? 83 A5", 1);
	hook::put<uint32_t>(unkLocation2, textureOverrideArraySize);

});
