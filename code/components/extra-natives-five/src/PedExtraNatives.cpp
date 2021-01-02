#include "StdInc.h"

#include <EntitySystem.h>

#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include "NativeWrappers.h"

#include <GameInit.h>

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, uint64_t* archetypeUnk)> getArchetype([]()
{
	return hook::get_call(hook::pattern("89 44 24 40 8B 4F 08 80 E3 01 E8").count(1).get(0).get<void>(10));
});

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

static hook::cdecl_stub<uint64_t(void* entity, uint64_t list)> g_extensionList_get([]()
{
	return hook::get_pattern("41 83 E0 1F 8B 44 81 08 44 0F A3 C0", -31);
});

static hook::cdecl_stub<uint16_t(uint32_t)> _getPedPersonalityIndex([]()
{
	return hook::get_call(hook::get_pattern("8B 86 B0 00 00 00 BB D5 46 DF E4 85 C0", 0x12));
});

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

static HookFunction initFunction([]()
{
	_id_CPedHeadBlendData = hook::get_address<uint64_t*>(hook::get_pattern("48 39 5E 38 74 1B 8B 15 ? ? ? ? 48 8D 4F 10 E8", 8));

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

	static std::list<std::tuple<uint32_t, uint16_t>> undoPersonalities;

	OnKillNetworkDone.Connect([]()
	{
		for (auto& [pedModel, personality] : undoPersonalities)
		{
			uint64_t index;
			auto archetype = getArchetype(pedModel, &index);

			// if is ped
			if (archetype && archetype->miType == 6)
			{
				*(uint16_t*)((char*)archetype + 0x14A) = personality;
			}
		}

		undoPersonalities.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PED_MODEL_PERSONALITY", [](fx::ScriptContext& context)
	{
		auto pedModel = context.GetArgument<uint32_t>(0);
		auto personality = context.GetArgument<uint32_t>(1);

		uint64_t index;
		auto archetype = getArchetype(pedModel, &index);

		// if is ped
		if (archetype && archetype->miType == 6)
		{
			auto index = _getPedPersonalityIndex(personality);

			if (index != 0xFFFF)
			{
				auto oldIndex = *(uint16_t*)((char*)archetype + 0x14A);
				*(uint16_t*)((char*)archetype + 0x14A) = index;

				undoPersonalities.push_front({ pedModel, oldIndex });
			}
		}
	});
});
