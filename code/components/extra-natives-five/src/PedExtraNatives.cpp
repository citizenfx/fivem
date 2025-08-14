#include "StdInc.h"

#include <EntitySystem.h>

#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include "NativeWrappers.h"

#include "atArray.h"

#include <GameInit.h>
#include <fxScripting.h>
#include <Resource.h>
#include <Hooking.Stubs.h>
#include <unordered_set>

class CPedHeadBlendData
{
public:
	void* vtable; // +0
	DWORD unk_0; // +8
	BYTE unk_1; // +12
	BYTE unk_2; // +13
	BYTE unk_3; // +14
	BYTE unk_4; // +15
	DWORD unk_5; // +16
	char pad_0[20]; // +20
	float shapeMix; // +40
	float skinMix; // +44
	float unknownMix; // +48
	DWORD unk_6; // +52
	float overlayAlpha[13]; // +56
	float overlayAlphaCopy[13]; // +108
	float faceFeature[20]; // +160
	uint8_t overlayColorId[13]; // +240
	uint8_t overlayHighlightId[13]; // +253
	uint8_t overlayColorType[13]; // +266
	uint8_t firstShapeId; // +279
	uint8_t secondShapeId; // +280
	uint8_t thirdShapeId; // +281
	uint8_t firstSkinId; // +282
	uint8_t secondSkinId; // +283
	uint8_t thirdSkinId; // +284
	uint8_t overlayValue[13]; // +285
	uint8_t paletteColorR[4]; // +298
	uint8_t paletteColorG[4]; // +302
	uint8_t paletteColorB[4]; // +306
	uint16_t eyeColour; // +310
	uint8_t hairColour; // +312
	uint8_t hairHighlight; // +313
	BYTE unk_7; // +314
	BYTE isPaletteUsed; // +315
	BYTE unk_8; // +316
	BYTE isParentBlend; // +317
	BYTE unk_9; // +318
};

static uint64_t* _id_CPedHeadBlendData;
static uintptr_t _baseClipsetLocation;
static uint32_t _pedSweatOffset;
static uint8_t _pedModelInfoOffset;
static uint32_t _pedPersonalityIndexOffset;
static uint8_t _pedHealthConfigHashOffset;
static float* _motionAimingTurnTransitionThresholdMin = nullptr;
static float* _motionAimingTurnTransitionThresholdMax = nullptr;

// There really isn't a deg2rad function anywhere (TODO: move this to a sensible spot?)
#define MATH_PI 3.14159265358979323846f
#define MATH_DEG2RAD (MATH_PI / 180.0f)
#define DEG2RAD(angle) ((angle) * MATH_DEG2RAD)

static hook::cdecl_stub<uint64_t(void* entity, uint64_t list)> g_extensionList_get([]()
{
	return hook::get_pattern("41 83 E0 1F 8B 44 81 08 44 0F A3 C0", -31);
});

static hook::cdecl_stub<uint16_t(uint32_t)> _getPedPersonalityIndex([]()
{
	return hook::get_call(hook::get_pattern("8B 86 B0 00 00 00 BB D5 46 DF E4 85 C0", 0x12));
});

static hook::cdecl_stub<bool(void*, int, int)> _doesPedComponentDrawableExist([]()
{
	return xbr::IsGameBuildOrGreater<2699>() ? hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 8B 57 48")) : nullptr;
});

struct PedPersonality
{
	uint32_t hash; // 0x00
	char padding[20];
	uint32_t healthConfigHash; // 0x18
	char paddingAfter[156];
};

static atArray<PedPersonality>* g_pedPersonalities;

static CPedHeadBlendData* GetPedHeadBlendData(fwEntity* entity)
{
	auto address = (char*)entity;
	if (*(BYTE*)(*(uint64_t*)(address + 32) + 646) & 2)
	{
		auto data = (CPedHeadBlendData*)g_extensionList_get(address + 16, *_id_CPedHeadBlendData);
		return data;
	}

	return nullptr;
}

static fwArchetype* GetPedModel(uint32_t pedModel)
{
	rage::fwModelId index;
	auto archetype = rage::fwArchetypeManager::GetArchetypeFromHashKey(pedModel, index);

	if (archetype && static_cast<CBaseModelInfo*>(archetype)->miType == 6)
	{
		return archetype;
	}

	return nullptr;
}

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

static std::vector<void*> clothPinRadiusScaleAddrs;

static void SetWetClothPinRadiusScale(float scale)
{
	for (auto& addr : clothPinRadiusScaleAddrs)
	{
		hook::put<float>(addr, scale);
	}
}

class CHealthConfigInfo
{
public:
	uint32_t hash;
	float defaultHealth;
	float defaultArmor;
	float defaultEndurance;
	float fatiguedHealthThreshold;
	float injuredHealthThreshold;
	float dyingHealthThreshold;
	float hurtHealthThreshold;
	float dogTakedownThreshold;
	float writheFromBulletDamageThreshold;
	bool meleeCardinalFatalAttackCheck;
	bool invincible;
};


static std::unordered_map<uint32_t, CHealthConfigInfo> g_healthConfigs;
static std::unordered_set<uint32_t> g_defaultHealthConfigHashes = {
	HashString("Strong"),
	HashString("Average"),
	HashString("Weak"),
	HashString("Animal"),
	HashString("Armour"),
	HashString("GULL"),
	HashString("Fish"),
	HashString("Rat"),
	HashString("Shark"),
	HashString("Humpback"),
};

static CHealthConfigInfo* (*g_origGetHealthInfo)(hook::FlexStruct*);
static CHealthConfigInfo* GetHealthInfo(hook::FlexStruct* pedPtr)
{
	if (!pedPtr)
		return nullptr;

	// Get model info: CPed + 0x20
	hook::FlexStruct* modelInfo = pedPtr->Get<hook::FlexStruct*>(_pedModelInfoOffset);
	if (!modelInfo)
		return nullptr;

	// Get personality index: modelInfo + 0x14A
	uint16_t personalityIndex = modelInfo->Get<uint16_t>(_pedPersonalityIndexOffset);
	if (personalityIndex >= g_pedPersonalities->GetCount())
		return nullptr;

	const auto& personality = g_pedPersonalities->Get(personalityIndex);
	hook::FlexStruct* personalityFS = (hook::FlexStruct*)&personality;
	
	uint32_t healthHash = personalityFS->Get<uint32_t>(_pedHealthConfigHashOffset);

	auto it = g_healthConfigs.find(healthHash);
	if (it != g_healthConfigs.end())
	{
		return &it->second;
	}

	const CHealthConfigInfo* info = g_origGetHealthInfo(pedPtr);
	if (info)
	{
		g_healthConfigs[info->hash] = *info;
		return &g_healthConfigs[info->hash];
	}
	else
	{
		trace("No health config found for ped personality hash: %08x\n", healthHash);
		return nullptr;
	}
}

static HookFunction initFunction([]()
{
	_id_CPedHeadBlendData = hook::get_address<uint64_t*>(hook::get_pattern("48 39 5E 38 74 1B 8B 15 ? ? ? ? 48 8D 4F 10 E8", 8));
	_baseClipsetLocation = (uintptr_t)hook::get_pattern("48 8B 42 ? 48 85 C0 75 05 E8");
	_pedSweatOffset = *hook::get_pattern<uint32_t>("72 04 41 0F 28 D0 F3 0F 10 8B", 10);

	_pedModelInfoOffset = *hook::get_pattern<uint8_t>("48 8B 41 ? 33 C9 0F BF 90", 3);
	_pedPersonalityIndexOffset = *hook::get_pattern<uint32_t>("0F BF 90 ? ? ? ? 48 8B 05 ? ? ? ? 48 69 D2 ? ? ? ? 44 8B 54", 3);
	_pedHealthConfigHashOffset = *hook::get_pattern<uint8_t>("0F BF 90 ? ? ? ? 48 8B 05 ? ? ? ? 48 69 D2 ? ? ? ? 44 8B 54", 25);

	g_origGetHealthInfo = hook::trampoline(hook::get_pattern("48 8B 41 20 33 C9 0F BF 90 ? ? ? ? 48 8B 05 ? ? ? ? 48 69 D2"), GetHealthInfo);

	g_pedPersonalities = hook::get_address<decltype(g_pedPersonalities)>(hook::get_call(hook::get_pattern<char>("8B 86 B0 00 00 00 BB D5 46 DF E4 85 C0", 0x12)) + 15, 3, 7);

	uint8_t* ptr = (uint8_t*)hook::get_pattern("0F 2F 35 ? ? ? ? 72 ? 0F 2F 35 ? ? ? ? 76 ? B0");
	_motionAimingTurnTransitionThresholdMin = hook::get_address<float*>(ptr, 3, 7);
	_motionAimingTurnTransitionThresholdMax = hook::get_address<float*>(ptr + 9, 3, 7);

	clothPinRadiusScaleAddrs.push_back(hook::get_pattern("F3 0F 10 05 ? ? ? ? 0F 2F C8 72 ? 0F 28 C8 80 89", 62)); // SetWetClothingHeight
	clothPinRadiusScaleAddrs.push_back(hook::get_pattern("C7 42 ? ? ? ? ? 48 83 C0", 3)); // ProcessWetClothingInWater
	clothPinRadiusScaleAddrs.push_back(hook::get_pattern("80 89 ? ? ? ? ? BA ? ? ? ? 48 81 C1", 30)); // IncWetClothing

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_EYE_COLOR", [=](fx::ScriptContext& context)
	{
		int result = -1;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (entity && entity->IsOfType<CPed>())
		{
			CPedHeadBlendData* data = GetPedHeadBlendData(entity);
			if (data != nullptr)
			{
				result = data->eyeColour;
			}
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_FACE_FEATURE", [=](fx::ScriptContext& context)
	{
		float result = 0.0f;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		int index = context.GetArgument<int>(1);

		if (index >= 0 && index < 20 && entity && entity->IsOfType<CPed>())
		{
			CPedHeadBlendData* data = GetPedHeadBlendData(entity);
			if (data != nullptr)
			{
				result = data->faceFeature[index];
			}
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_HEAD_OVERLAY_DATA", [=](fx::ScriptContext& context)
	{
		bool result = false;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		int index = context.GetArgument<int>(1);

		if (index >= 0 && index <= 13 && entity && entity->IsOfType<CPed>())
		{
			CPedHeadBlendData* data = GetPedHeadBlendData(entity);
			if (data != nullptr)
			{
				*context.GetArgument<int*>(2) = data->overlayValue[index];
				*context.GetArgument<int*>(3) = data->overlayColorType[index];
				*context.GetArgument<int*>(4) = data->overlayColorId[index];
				*context.GetArgument<int*>(5) = data->overlayHighlightId[index];
				*context.GetArgument<float*>(6) = data->overlayAlpha[index];

				result = true;
			}
		}

		context.SetResult<bool>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_HAIR_COLOR", [=](fx::ScriptContext& context)
	{
		int result = -1;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (entity && entity->IsOfType<CPed>())
		{
			CPedHeadBlendData* data = GetPedHeadBlendData(entity);
			if (data != nullptr)
			{
				result = data->hairColour;
			}
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_HAIR_HIGHLIGHT_COLOR", [=](fx::ScriptContext& context)
	{
		int result = -1;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (entity && entity->IsOfType<CPed>())
		{
			CPedHeadBlendData* data = GetPedHeadBlendData(entity);
			if (data != nullptr)
			{
				result = data->hairHighlight;
			}
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_MOVEMENT_CLIPSET", [=](fx::ScriptContext& context)
	{
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		context.SetResult<int>(-1);

		if (!entity || !entity->IsOfType<CPed>())
		{
			return;
		}

		if (!_baseClipsetLocation)
		{
			return;
		}

		static uint32_t offset_unk = *(uint32_t*)(_baseClipsetLocation - 0xB);
		static uint32_t offset_ptrCTaskTreeMotion = *(uint32_t*)(_baseClipsetLocation - 0x4);
		static uint8_t offset_ptrCTaskMotionPed = *(uint8_t*)(_baseClipsetLocation + 0x3);

		uint64_t unk = *(uint64_t*)((uint64_t)entity + offset_unk);
		if (!unk)
		{
			return;
		}

		uint64_t ptrCTaskTreeMotion = *(uint64_t*)(unk + offset_ptrCTaskTreeMotion);
		if (!ptrCTaskTreeMotion)
		{
			return;
		}

		uint64_t ptrCTaskMotionPed = *(uint64_t*)(ptrCTaskTreeMotion + offset_ptrCTaskMotionPed);
		if (!ptrCTaskMotionPed)
		{
			return;
		}

		// 0xE0 hasn't changed in forever
		// How to update:
		// 41 C0 E0 ? F3 0F 11 91 ? ? ? ? 44 08 81 + Offset 21 (dec)
		context.SetResult<int>(*(int32_t*)(ptrCTaskMotionPed + 0xE0));
	});

	static std::map<uint32_t, uint16_t> initialPersonalities;
	static std::list<std::tuple<uint32_t, uint16_t>> undoPersonalities;

	static std::unordered_map<uint32_t, CHealthConfigInfo> initialHealthConfigs;
	static std::unordered_map<uint32_t, uint32_t> undoHealthConfigs;

	OnKillNetworkDone.Connect([]()
	{
		for (auto& [pedModel, personality] : undoPersonalities)
		{
			auto archetype = GetPedModel(pedModel);

			// if is ped
			if (archetype)
			{
				*(uint16_t*)((char*)archetype + 0x14A) = personality;
			}
		}

		for (auto& [pedModel, savedHealthConfig] : undoHealthConfigs)
		{
			auto archetype = GetPedModel(pedModel);

			if (archetype)
			{
				auto personalityIndex = reinterpret_cast<hook::FlexStruct*>(archetype)->Get<uint16_t>(_pedPersonalityIndexOffset);
				if (personalityIndex != 0xFFFF && personalityIndex < g_pedPersonalities->GetCount())
				{
					auto& personality = g_pedPersonalities->Get(personalityIndex);
					if (personality.healthConfigHash != savedHealthConfig)
					{
						personality.healthConfigHash = savedHealthConfig;
					}
				}
			}
		}

		g_healthConfigs.clear();

		undoHealthConfigs.clear();
		undoPersonalities.clear();
		SetWetClothPinRadiusScale(0.3f);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PED_MODEL_PERSONALITY", [](fx::ScriptContext& context)
	{
		auto pedModel = context.GetArgument<uint32_t>(0);
		auto personality = context.GetArgument<uint32_t>(1);

		auto archetype = GetPedModel(pedModel);

		// if is ped
		if (archetype)
		{
			auto index = _getPedPersonalityIndex(personality);

			if (index != 0xFFFF)
			{
				auto oldIndex = *(uint16_t*)((char*)archetype + 0x14A);
				*(uint16_t*)((char*)archetype + 0x14A) = index;

				undoPersonalities.push_front({ pedModel, oldIndex });

				if (initialPersonalities.find(pedModel) == initialPersonalities.end())
				{
					initialPersonalities.emplace(pedModel, oldIndex);
				}
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("RESET_PED_MODEL_PERSONALITY", [](fx::ScriptContext& context)
	{
		auto pedModel = context.GetArgument<uint32_t>(0);

		auto archetype = GetPedModel(pedModel);

		// if is ped
		if (archetype)
		{
			if (auto it = initialPersonalities.find(pedModel); it != initialPersonalities.end())
			{
				*(uint16_t*)((char*)archetype + 0x14A) = it->second;
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_MODEL_PERSONALITY", [](fx::ScriptContext& context)
	{
		auto pedModel = context.GetArgument<uint32_t>(0);

		auto archetype = GetPedModel(pedModel);

		uint32_t result = 0;

		// if is ped
		if (archetype)
		{
			auto index = *(uint16_t*)((char*)archetype + 0x14A);
			if (index < g_pedPersonalities->GetCount())
			{
				result = g_pedPersonalities->Get(index).hash;
			}
		}

		context.SetResult<uint32_t>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_SWEAT", [](fx::ScriptContext& context)
	{
		float sweat = 0.0f;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (entity && entity->IsOfType<CPed>())
		{
			sweat = *(float*)((char*)entity + _pedSweatOffset);
		}

		context.SetResult<float>(sweat);
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_PED_COMPONENT_VARIATION_GEN9_EXCLUSIVE", [](fx::ScriptContext& context)
	{
		bool result = false;

		if (xbr::IsGameBuildOrGreater<2699>())
		{
			fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

			if (entity && entity->IsOfType<CPed>())
			{
				auto componentIndex = context.GetArgument<int>(1);

				if (componentIndex >= 0 && componentIndex < 12)
				{
					auto drawableIndex = context.GetArgument<int>(2);

					result = !_doesPedComponentDrawableExist(entity, componentIndex, drawableIndex);	
				}
			}
		}

		context.SetResult<bool>(result);
	});


	// Purpose: The game's default values for these make shooting while traveling Left quite a bit slower than shooting while traveling right
	fx::ScriptEngine::RegisterNativeHandler("SET_PED_TURNING_THRESHOLDS", [](fx::ScriptContext& context) 
	{
		// Default Min: -45 Degrees
		// Default Max: 135 Degrees
		// 
		//        \ ,- ~ ||~ - ,
		//     , ' \    x   x    ' ,
		//   ,      \    x    x   x  ,
		//  ,         \  x     x      ,
		// ,            \     x    x  ,
		// ,              \      x    ,
		// ,                \   x     ,
		//  ,                 \   x x ,
		//   ,                  \  x ,
		//     ,                 \, '
		//       ' - , _ _ _ ,  '  \
		// If the transition angle is within the shaded portion (x), there will be no transition(Quicker)
		// The angle corresponds to where you are looking(North on the circle) vs. the heading of your character.
		// Note: For some reason, the transition spin is only clockwise.

		float min = context.GetArgument<float>(0);
		float max = context.GetArgument<float>(1);

		*_motionAimingTurnTransitionThresholdMin = DEG2RAD(min);
		*_motionAimingTurnTransitionThresholdMax = DEG2RAD(max);

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			resource->OnStop.Connect([]()
			{
				*_motionAimingTurnTransitionThresholdMin = DEG2RAD(-45.0f);
				*_motionAimingTurnTransitionThresholdMax = DEG2RAD(135.0f);
			});
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WET_CLOTH_PIN_RADIUS_SCALE", [](fx::ScriptContext& context)
	{
		float scale = context.GetArgument<float>(0);
		float newScale = std::isnan(scale) ? 0.3f : std::clamp(scale, 0.0f, 1.0f);
		SetWetClothPinRadiusScale(newScale);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_MODEL_HEALTH_CONFIG", [](fx::ScriptContext& context)
	{
		auto pedModel = context.GetArgument<uint32_t>(0);
		auto archetype = GetPedModel(pedModel);
		uint32_t healthHash = 0;

		if (archetype)
		{
			auto personalityIndex = reinterpret_cast<hook::FlexStruct*>(archetype)->Get<uint16_t>(_pedPersonalityIndexOffset);
			if (personalityIndex < g_pedPersonalities->GetCount())
			{
				healthHash = g_pedPersonalities->Get(personalityIndex).healthConfigHash;
			}
		}

		context.SetResult<uint32_t>(healthHash);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PED_MODEL_HEALTH_CONFIG", [](fx::ScriptContext& context)
	{
		auto pedModel = context.GetArgument<uint32_t>(0);
		std::string healthConfigName = context.GetArgument<const char*>(1);
		uint32_t healthConfigHash = HashString(healthConfigName);

		auto archetype = GetPedModel(pedModel);

		if (archetype)
		{
			auto personalityIndex = reinterpret_cast<hook::FlexStruct*>(archetype)->Get<uint16_t>(_pedPersonalityIndexOffset);
			if (personalityIndex != 0xFFFF)
			{
				auto& personality = g_pedPersonalities->Get(personalityIndex);
				auto oldHash = personality.healthConfigHash;

				personality.healthConfigHash = healthConfigHash;

				// save only first change
				if (undoHealthConfigs.find(pedModel) == undoHealthConfigs.end())
				{
					undoHealthConfigs[pedModel] = oldHash;
				}
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("ADD_HEALTH_CONFIG", [](fx::ScriptContext& context)
	{
		std::string configName = context.GetArgument<const char*>(0);
		uint32_t nameHash = HashString(configName);

		// Disallow if this hash belongs to a default config
		if (g_defaultHealthConfigHashes.find(nameHash) != g_defaultHealthConfigHashes.end())
		{
			trace("Cannot create a new health config: '%s' is a reserved default config name.\n", configName.c_str());
			return;
		}

		if (g_healthConfigs.find(nameHash) != g_healthConfigs.end())
		{
			trace("Cannot create a new health config: '%s' already exists.\n", configName.c_str());
			return;
		}

		float defaultHealth = context.GetArgument<float>(1);
		float defaultArmor = context.GetArgument<float>(2);
		float defaultEndurance = context.GetArgument<float>(3);
		float fatiguedHealthThreshold = context.GetArgument<float>(4);
		float injuredHealthThreshold = context.GetArgument<float>(5);
		float dyingHealthThreshold = context.GetArgument<float>(6);
		float hurtHealthThreshold = context.GetArgument<float>(7);
		float dogTakedownThreshold = context.GetArgument<float>(8);
		float writheFromBulletDamageThreshold = context.GetArgument<float>(9);
		bool meleeCardinalFatalAttackCheck = context.GetArgument<bool>(10);
		bool isInvincible = context.GetArgument<bool>(11);

		CHealthConfigInfo newConfig{};
		newConfig.hash = nameHash;
		newConfig.defaultHealth = defaultHealth;
		newConfig.defaultArmor = defaultArmor;
		newConfig.defaultEndurance = defaultEndurance;
		newConfig.fatiguedHealthThreshold = fatiguedHealthThreshold;
		newConfig.injuredHealthThreshold = injuredHealthThreshold;
		newConfig.dyingHealthThreshold = dyingHealthThreshold;
		newConfig.hurtHealthThreshold = hurtHealthThreshold;
		newConfig.dogTakedownThreshold = dogTakedownThreshold;
		newConfig.writheFromBulletDamageThreshold = writheFromBulletDamageThreshold;
		newConfig.meleeCardinalFatalAttackCheck = meleeCardinalFatalAttackCheck;
		newConfig.invincible = isInvincible;

		g_healthConfigs.emplace(nameHash, newConfig);
	});

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_HEALTH_CONFIG", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t nameHash = HashString(name);

		// Disallow if this hash belongs to a default config
		if (g_defaultHealthConfigHashes.find(nameHash) != g_defaultHealthConfigHashes.end())
		{
			trace("Cannot remove health config: '%s' is a reserved default config name.\n", name.c_str());
			return;
		}

		auto it = g_healthConfigs.find(nameHash);
		if (it != g_healthConfigs.end())
		{
			g_healthConfigs.erase(it);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_DEFAULT_HEALTH", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float newValue = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.defaultHealth = newValue;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_DEFAULT_ARMOR", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float value = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.defaultArmor = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_DEFAULT_ENDURANCE", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float value = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.defaultEndurance = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_FATIGUED_THRESHOLD", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float value = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.fatiguedHealthThreshold = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_INJURED_THRESHOLD", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float value = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.injuredHealthThreshold = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_DYING_THRESHOLD", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float value = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.dyingHealthThreshold = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_HURT_THRESHOLD", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float value = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.hurtHealthThreshold = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_DOG_TAKEDOWN_THRESHOLD", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float value = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.dogTakedownThreshold = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_WRITHE_FROM_BULLET_THRESHOLD", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		float value = context.GetArgument<float>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.writheFromBulletDamageThreshold = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_MELEE_FATAL_ATTACK", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		bool value = context.GetArgument<bool>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.meleeCardinalFatalAttackCheck = value;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HEALTH_CONFIG_INVINCIBLE", [](fx::ScriptContext& context)
	{
		std::string name = context.GetArgument<const char*>(0);
		uint32_t configHash = HashString(name);
		bool value = context.GetArgument<bool>(1);

		auto it = g_healthConfigs.find(configHash);
		if (it != g_healthConfigs.end())
		{
			it->second.invincible = value;
		}
	});
});
