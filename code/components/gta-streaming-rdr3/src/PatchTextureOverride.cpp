#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>
#include <scrEngine.h>
#include <MinHook.h>
#include "Hooking.Stubs.h"
#include <EntitySystem.h>

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
	uint32_t m_colorType; // 76
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
	uint16_t m_generation; // 1888
	uint16_t unk_143EC3622; // 1890
	uint8_t m_normalUnkValue; // 1892
	uint8_t m_materialUnkValue; // 1893
	uint8_t m_UnkValue; // 1894
	uint8_t m_UnkValue2; // 1895
	uint8_t m_textureOverrideOwnerId; // 1896
	uint8_t m_flags; // 1897;
	char pad[2]; // 1898-1899
	uint32_t m_categoryHash; // we use this padding as categoryHash
};

struct PatternPair
{
	std::string_view pattern;
	int offset;
	intptr_t address;
	int count;
	int id;
};

struct CSyncDataBase
{
	void* __vftable;
	uint8_t m_type;
	char pad[7];
	void* m_logger;
};

struct AppearanceComponentOverlay
{
	uint32_t m_unkType; // 0
	uint32_t m_albedoHash; // 4
	uint32_t m_normalHash; // 8
	uint32_t m_materialHash; // 12
	uint32_t m_modTexture; // 16
	uint32_t m_colorType; // 20
	float m_roughness; // 24
	float m_alpha; // 28
	float m_modAlpha; // 32
	uint8_t m_hasModLayers; // 36
	uint8_t m_tint1; // 37
	uint8_t m_tint2; // 38
	uint8_t m_tint3; // 39
	uint8_t m_albedoTextureIndex; // 40
	char pad[3]; // 41
};

struct AppearanceComponent
{
	AppearanceComponentOverlay overlays[16];
	uint16_t overlaysCount; // 704 base layer its also counted!
	int32_t componentHash;
};

struct UnkMetaPedUsedComponent
{
	UnkMetaPedUsedComponent* m_next;
	void* unk_8;
	void* unk_10;
	uint32_t componentHash;
	uint32_t unk_1C;
	void* unk_20;
	uint32_t unk_28;
	uint8_t unk_2C;
	char pad_2D[3];
};

struct UnkPedDrawHandlerMetaPed
{
	char pad[2832];
	UnkMetaPedUsedComponent* m_usedComponents;
	uint16_t m_usedComponentCount;
	uint16_t m_usedComponentSize;
};

struct CPedDrawHandler
{
	char pad[1872];
	UnkPedDrawHandlerMetaPed* m_metaPedData;
};

static textureOverride* textureOverrideArray;
static uint32_t textureOverrideArraySize = 384;
static intptr_t textureCountLocation;

static hook::cdecl_stub<bool(uint32_t)> doesTextureOverrideExists([]()
{
	return hook::get_pattern("0F B7 C1 3B 05 ? ? ? ? 7D ? 0F B7 C1 4C 8D 05 ? ? ? ? 48 69 D0");
});

static hook::cdecl_stub<bool(CPed* ped)> hasPlayerComponent([]()
{
	return hook::get_pattern("0F 95 C0 C3 48 83 EC ? 83 F9", -40);
}); 

static hook::cdecl_stub<uint32_t*(uint32_t*, UnkMetaPedUsedComponent*, uint32_t)> getCategoryHash([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 8B 44 24 ? 85 C0 74 ? 44 0F B7 8F"));
});

static hook::cdecl_stub<CPedDrawHandler*(CPed* ped)> getDrawHandler([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B 98 ? ? ? ? EB ? 33 DB 48 85 DB 75 ? 8D 43"));
});

static hook::cdecl_stub<void(int64_t*, uint32_t*, uint16_t, uint32_t*, uint8_t*)> sub_140AAE5AC([]()
{
	return hook::get_pattern("48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 8B 1A");
});

static hook::cdecl_stub<void(textureOverride*, bool)> clearPedTexture([]()
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B A9 ? ? ? ? 33 F6 40 8A FA");
});

static hook::cdecl_stub<uint32_t(uint32_t, uint32_t*, uint32_t*, uint32_t*)> createPedTextureOverrideOverlay([]()
{
	return hook::get_pattern("4C 8B DC 53 55 56 57 41 54 41 56 41 57 48 83 EC ? 4C 8D 25");
});

static hook::cdecl_stub<uint32_t(uint32_t*, uint32_t*, uint32_t*)> initPedTextureOverride([]()
{
	return hook::get_pattern("4C 8B DC 49 89 5B ? 49 89 73 ? 49 89 7B ? 55 48 8B EC 48 83 EC ? 48 83 3D");
});

// hacky
static hook::cdecl_stub<uint32_t(fwEntity*)> getScriptGuidForEntity([]()
{
	return hook::get_pattern("32 DB E8 ? ? ? ? 48 85 C0 75 ? 8A 05", -35);
});

static uint32_t getCategoryHashFromComponent(CPed* ped, uint32_t targetHash)
{
	auto pedDrawHandler = getDrawHandler(ped);
	if (!pedDrawHandler)
		return 0;

	auto metaPedData = pedDrawHandler->m_metaPedData;
	for (int i = 0; i < metaPedData->m_usedComponentCount; ++i)
	{
		auto entry = metaPedData->m_usedComponents[i].m_next;
		if (entry && entry->componentHash == targetHash)
		{
			uint32_t categoryHash;
			getCategoryHash(&categoryHash, entry, 0);
			return categoryHash;
		}
	}
	return 0;
}

static bool searchForTexture(uint8_t textureOwnerId, uint32_t categoryHash, AppearanceComponent* appearance, uint32_t* textureId)
{

	int g_pedTextureOverridesCount = *(intptr_t*)textureCountLocation;

	if (g_pedTextureOverridesCount <= 0)
	{
		*textureId = -1;
		return 0;
	}

	for (int i = 0; i < textureOverrideArraySize; i++)
	{
		auto& entry = textureOverrideArray[i];

		if (entry.m_textureOverrideOwnerId == textureOwnerId && entry.m_categoryHash == categoryHash)
		{

			// There should be check for identical copy but will be never used because WE alway clear all components textures before applying new ones.
			//

			// return texture id and clear
			*textureId = i | (entry.m_generation << 16);
			return 1;
		}
	}

	*textureId = -1;
	return 0;
}

static void clearAllPedTextures(CPed* ped)
{
	if (!ped)
		return;

	if (!hasPlayerComponent(ped))
		return;

	// hacky shit there for now
	auto sguid = getScriptGuidForEntity(ped);
	uint8_t textureOwnerId = NativeInvoke::Invoke<0x6C0E2E0125610278, uint32_t>(sguid);

	for (int i = 0; i < textureOverrideArraySize; i++)
	{
		auto& entry = textureOverrideArray[i];

		if (entry.m_textureOverrideOwnerId == textureOwnerId)
		{
			auto textureId = i | (entry.m_generation << 16);
			if (doesTextureOverrideExists(textureId))
			{
				clearPedTexture(&entry, 0);
				
			}
		}
	}
}

static uint32_t acquireTexture(uint32_t* albedoHash, uint32_t* normalHash, uint32_t* materialHash, uint8_t textureOwnerId, AppearanceComponent* appearance, uint8_t* outFlag, CPed* ped)
{

	if (!ped)
		return -1;

	uint32_t categoryHash = getCategoryHashFromComponent(ped, appearance->componentHash);
	*outFlag = 0;

	int64_t unk_1 = 0; 
	uint32_t unk_2 = 0;
	uint8_t unk_3;
	sub_140AAE5AC(&unk_1, albedoHash, 0, &unk_2, &unk_3);

	if (unk_1 == -1)
		return -1;


	uint32_t textureId = -1;
	if (searchForTexture(textureOwnerId, categoryHash, appearance, &textureId))
	{
		uint16_t textureIndex = textureId & 0xFFFF;
		auto& entry = textureOverrideArray[textureIndex];	

		clearPedTexture(&entry, 0);
		++entry.m_generation;

		auto newTextureId = createPedTextureOverrideOverlay(textureIndex, albedoHash, normalHash, materialHash);
		entry.m_categoryHash = categoryHash;
		*outFlag = 1;
		return newTextureId;
	}
	else if (doesTextureOverrideExists(textureId))
	{
		*outFlag = 0;
		return textureId;
	}
	else
	{
		auto newTextureId = initPedTextureOverride(albedoHash, normalHash, materialHash);
		uint16_t textureIndex = newTextureId & 0xFFFF;
		if (textureIndex >= 0 && textureIndex < textureOverrideArraySize)
			textureOverrideArray[textureIndex].m_categoryHash = categoryHash;
		*outFlag = 1;
		return newTextureId;
	}

	return 0;
}

static void RelocateAbsolute(std::initializer_list<PatternPair> list)
{
	for (const auto& entry : list)
	{
		auto location = hook::pattern(entry.pattern).count(entry.count).get(entry.id).get<int32_t>(entry.offset);
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

static HookFunction hookFunction([]()
{
	textureOverrideArray = reinterpret_cast<textureOverride*>(hook::AllocateStubMemory(sizeof(textureOverride) * textureOverrideArraySize));

	static struct : jitasm::Frontend
	{
		intptr_t jumpBack;
		void Init(intptr_t location)
		{
			this->jumpBack = location + 8;
		}

		void InternalMain() override
		{
			mov(rcx, rsi);
			mov(rax, reinterpret_cast<uintptr_t>(&clearAllPedTextures));
			call(rax);


			xor(bl, bl);
			mov(byte_ptr[rbp + 0x5CC0 + 0x28], bl);

			mov(rax, jumpBack);
			jmp(rax);

		}
	} clearStub;

	auto clearLocation = hook::get_pattern("32 DB 88 9D ? ? ? ? 38 9F ? ? ? ? 0F 86");
	hook::nop(clearLocation, 8);

	clearStub.Init((intptr_t)clearLocation);
	hook::jump(clearLocation, clearStub.GetCode());


	static struct : jitasm::Frontend
	{
		intptr_t retSuccess;
		intptr_t retFail;
		void Init(intptr_t location, intptr_t retFail)
		{
			this->retSuccess = location + 23;
			this->retFail = retFail;
		}

		void InternalMain() override
		{
			mov(ptr[rsp + 0x30], rsi);
			mov(rax, reinterpret_cast<uintptr_t>(&acquireTexture));
			call(rax);

			cmp(byte_ptr[rbp + 0x5CC0 + 0x20], 0);

			mov(r15d, eax);
			jz("fail");

			mov(ecx, eax);

			mov(rax, retSuccess);
			jmp(rax);

			L("fail");
			mov(rax, retFail);
			jmp(rax);

		}
	} getTextureIdStub;

	auto acquireTextureLocation = hook::get_pattern("4C 89 6C 24 ? C6 85", 12);
	hook::nop(acquireTextureLocation, 23);

	auto retFail = hook::get_pattern("41 8B 85 ? ? ? ? 4C 8D 85 ? ? ? ? 45 8A CC 89 85 ? ? ? ? 48 8B D6 41 8B CF E8 ? ? ? ? FE C3");
	
	getTextureIdStub.Init((intptr_t)acquireTextureLocation, (intptr_t)retFail);
	hook::jump(acquireTextureLocation, getTextureIdStub.GetCode());




	auto maxSerializeComponentCountLocation = hook::get_pattern<char>("41 B0 ? B2 ? 48 8B CF E8 ? ? ? ? 48 8B CF 48 89 AF", 2);
	hook::put<uint8_t>(maxSerializeComponentCountLocation, 3); // set max serialize component count value

	hook::nop(hook::get_pattern("41 0F 47 C7 44 88 B3"), 4); // nop max serialize components check  if > 1 then set 1 

	auto serializeBitsCountLocation = hook::get_pattern<char>("41 B8 ? ? ? ? 44 8A 8E ? ? ? ? 49 8B D6 88 5C 24 ? 48 8B CF 48 89 5C 24 ? 48 89 5C 24 ? FF 90 ? ? ? ? 41 38 1E", 2);
	hook::put<uint32_t>(serializeBitsCountLocation, 3); // 3 bits // extend serialize bits


	  
	fx::ScriptEngine::RegisterNativeHandler("REMOVE_TEXTURE", [](fx::ScriptContext& context)
	{
		uint32_t fullTextureId = context.GetArgument<uint32_t>(0);
		uint16_t textureIndex = fullTextureId & 0xFFFF;

		if (textureIndex >= 0 && textureIndex < textureOverrideArraySize)
		{
			if (doesTextureOverrideExists(fullTextureId))
			{
				uint16_t textureId = fullTextureId & 0xFFFF;
				textureOverrideArray[textureId].unk_143EC3622 = 0;
			}
		}
		
	});

    fx::ScriptEngine::RegisterNativeHandler("DOES_TEXTURE_EXIST", [](fx::ScriptContext& context)
	{
		uint32_t fullTextureId = context.GetArgument<uint32_t>(0);
		uint16_t textureIndex = fullTextureId & 0xFFFF;

		if (textureIndex >= 0 && textureIndex < textureOverrideArraySize)
		{
			bool exist = doesTextureOverrideExists(fullTextureId);
			context.SetResult<bool>(exist);
		}
		else
		{
			context.SetResult<bool>(false);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_TEXTURE", [](fx::ScriptContext& context)
	{
		uint32_t playerId = context.GetArgument<uint32_t>(0);
		uint32_t categoryHash = context.GetArgument<uint32_t>(1);

		uint32_t textureId = -1;

		for (int i = 0; i < textureOverrideArraySize; i++)
		{
			auto& entry = textureOverrideArray[i];

			bool exists = (entry.m_flags & 4) != 0 || (entry.m_flags & 16) != 0;

			if (!exists)
				continue;

			if (entry.m_textureOverrideOwnerId == (uint8_t)playerId && entry.m_categoryHash == categoryHash)
			{
				uint32_t fullTextureId = i | (entry.m_generation << 16);
				textureId = fullTextureId;
				break;
			}
		}

		context.SetResult<uint32_t>(textureId);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TEXTURE_OVERLAYS", [](fx::ScriptContext& context)
	{
		uint32_t fullTextureId = context.GetArgument<uint32_t>(0);
		uint16_t textureIndex = fullTextureId & 0xFFFF;

		int count = 0;
		if (textureIndex < textureOverrideArraySize && doesTextureOverrideExists(fullTextureId))
			count = textureOverrideArray[textureIndex].m_overlaysCount;

		context.SetResult<int>(count);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_FREE_TEXTURES", [](fx::ScriptContext& context)
	{

		// the count at textureCountLocation is how many entries the game initialised, not how many
		// are taken, so count the free ones the way the allocator does
		int free = 0;

		for (uint32_t i = 0; i < textureOverrideArraySize; i++)
		{
			bool exists = (textureOverrideArray[i].m_flags & 4) != 0 || (textureOverrideArray[i].m_flags & 16) != 0;
			if (!exists)
			{
				free++;
			}
		}

		context.SetResult<int>(free);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TEXTURE_OWNER", [](fx::ScriptContext& context)
	{
		uint32_t fullTextureId = context.GetArgument<uint32_t>(0);
		uint16_t textureIndex = fullTextureId & 0xFFFF;

		int textureOwner = -1;
		if (textureIndex < textureOverrideArraySize && doesTextureOverrideExists(fullTextureId))
			textureOwner = textureOverrideArray[textureIndex].m_textureOverrideOwnerId;

		context.SetResult<int>(textureOwner);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TEXTURE_COMPONENT_CATEGORY", [](fx::ScriptContext& context)
	{
		uint32_t fullTextureId = context.GetArgument<uint32_t>(0);
		uint16_t textureIndex = fullTextureId & 0xFFFF;

		uint32_t categoryHash = 0;
		if (textureIndex < textureOverrideArraySize && doesTextureOverrideExists(fullTextureId))
		{
			categoryHash = textureOverrideArray[textureIndex].m_categoryHash;
		}

		context.SetResult<uint32_t>(categoryHash);
	});

	// textures built by a local client never reach acquireTexture, so record the component here as well
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		constexpr uint64_t applyTextureToPed = 0x0B46E25761519058;

		auto handler = fx::ScriptEngine::GetNativeHandler(applyTextureToPed);

		if (!handler)
		{
			return;
		}

		fx::ScriptEngine::RegisterNativeHandler(applyTextureToPed, [handler](fx::ScriptContext& context)
		{
			handler(context);

			uint32_t categoryHash = context.GetArgument<uint32_t>(1);
			uint32_t fullTextureId = context.GetArgument<uint32_t>(2);
			uint16_t textureIndex = fullTextureId & 0xFFFF;

			if (textureIndex < textureOverrideArraySize)
			{
				textureOverrideArray[textureIndex].m_categoryHash = categoryHash;
			}
		});
	});


	RelocateAbsolute({
	// Albedo
	{ "48 8D 86 ? ? ? ? FF C1", 3, (intptr_t)&textureOverrideArray->m_albedoTexBlendTextureIntermediatePtr, 1, 0 },
	{ "4C 8D 8E ? ? ? ? C6 44 24 ? ? 48 8D 8E", 3, (intptr_t)&textureOverrideArray->m_albedoTexBlendTexturePtr, 1, 0 },
	{ "4C 8D 8E ? ? ? ? C6 44 24 ? ? 48 8D 8E", 15, (intptr_t)&textureOverrideArray->m_albedoUAVOT, 1, 0 },


	// older build
	// 48 8D 8E ? ? ? ? C6 44 24 ? ? 48 8D 86 ? ? ? ? 48 03 CB 3 i  15
	// C6 84 33 ? ? ? ? ? 48 89 54 24 3
	// 4C 8D 8E ? ? ? ? 48 03 C3 3
	// Material
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8", -42, (intptr_t)&textureOverrideArray->m_materialUnkValue, 2, 0 },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8", -29, (intptr_t)&textureOverrideArray->m_materialUAVOT, 2, 0 },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8", -17, (intptr_t)&textureOverrideArray->m_materialTexBlendTextureIntermediatePtr, 2, 0 },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8", 3, (intptr_t)&textureOverrideArray->m_materialTexBlendTexturePtr, 2, 0 },


	// older build	
	// C6 84 33 ? ? ? ? ? E9 3
	// 48 8D 8E ? ? ? ? C6 44 24 ? ? 48 8D 86 ? ? ? ? E9 3 i 15
	// 4C 8D 8E ? ? ? ? E9 3
	// Normal
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8", -42, (intptr_t)&textureOverrideArray->m_normalUnkValue, 2, 1 },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8", -29, (intptr_t)&textureOverrideArray->m_normalUAVOT, 2, 1 },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8", -17, (intptr_t)&textureOverrideArray->m_normalTexBlendTextureIntermediatePtr, 2, 1 },
	{ "4C 8D 8E ? ? ? ? 48 03 C3 55 8B 0C 24 5D 4C 03 CB 48 89 44 24 ? 41 B8", 3, (intptr_t)&textureOverrideArray->m_normalTexBlendTexturePtr, 2, 1 },

	
	// Other
	{ "C6 84 33 ? ? ? ? ? 48 83 EF", 3, (intptr_t)&textureOverrideArray->m_UnkValue, 1, 0 },
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
		{ "48 83 EC ? 41 8A F0 8B DA 8B F9 E8 ? ? ? ? 84 C0", 26 },
		{ "48 8D 05 ? ? ? ? 41 8A 84 06", 3 },
		{ "48 8D 2D ? ? ? ? 48 69 D8", 3 },
		{ "48 8D 0D ? ? ? ? 8B 55 ? 48 83 C1", 3 },
		{ "48 8D 0D ? ? ? ? 44 8B 05 ? ? ? ? 48 83 C1 ? 48 03 CE 41 B9 ? ? ? ? 8B D7", 3 },
		{ "48 8D 0D ? ? ? ? 44 8B 05 ? ? ? ? 48 83 C1 ? 48 03 CE 41 B9 ? ? ? ? 8B D3", 3 },
		{ "48 8D 1D ? ? ? ? 89 44 1E", 3 },
		{ "48 83 EC ? 41 8A F1 8B DA", 29 },
		{ "48 8D 05 ? ? ? ? 41 39 8C 03", 3 },
		{ "0F 8C ? ? ? ? 48 8D 05 ? ? ? ? 41 0F B7 84 03", 9 },
		{ "E9 ? ? ? ? 48 8D 05 ? ? ? ? 41 0F B7 84 03", 8 },
		{ "0F B7 C3 48 8D 15 ? ? ? ? 48 69 C8 ? ? ? ? 8A 84 11", 6 },
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
	patchPointer("48 8D 05 ? ? ? ? 41 8B C8 48 69 D1", 3, &textureOverrideArray->m_generation);
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
		intptr_t textureCount;
		void Init(intptr_t location, intptr_t textureCount)
		{
			this->failLocation = location + 43;
			this->textureCount = textureCount;
		}

		void InternalMain() override
		{
			cmp(eax, textureOverrideArraySize);
			jge("fail");

			add(eax, r10d);
			mov(r11, textureCount);
			mov(dword_ptr[r11], eax);
			dec(eax);
			ret();

			L("fail");
			mov(rax, failLocation);
			jmp(rax);
		}
	} allocationStub;

	textureCountLocation = hook::get_address<intptr_t>(hook::get_pattern("48 63 05 ? ? ? ? 45 33 C0", 3));
	auto allocatePedTextureOverrideLocation = hook::get_pattern("83 F8 ? 7D ? 41 03 C2");
	hook::nop(allocatePedTextureOverrideLocation, 17);

	allocationStub.Init(reinterpret_cast<intptr_t>(allocatePedTextureOverrideLocation), textureCountLocation);
	hook::jump(allocatePedTextureOverrideLocation, allocationStub.GetCode());

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

	auto clearPedOverlaysTableLocation = hook::get_pattern<char>("E9 ? ? ? ? CC 48 89 5C 24 ? 57 48 83 EC ? BF ? ? ? ? 48 8D 1D ? ? ? ? 48 8D 8B", -48);
	hook::put<uint32_t>(clearPedOverlaysTableLocation, textureOverrideArraySize);

	auto unkLocation = hook::get_pattern<char>("48 83 C4 ? 5F C3 CC CC C2 ? ? CC C2 ? ? CC C2 ? ? CC 48 83 EC", -53);
	hook::put<uint32_t>(unkLocation, textureOverrideArraySize);

	// there is nothing else to hook around because its function fragment
	auto unkLocation2 = hook::get_pattern<char>("BF 20 00 00 00 BD 00 02 00 00 E9", 1);
	hook::put<uint32_t>(unkLocation2, textureOverrideArraySize);

});