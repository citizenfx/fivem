
#include "StdInc.h"

#include <atArray.h>
#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include "NativeWrappers.h"

static hook::cdecl_stub<uint64_t(void* entity, uint64_t list)> g_extensionList_get([]()
{
	return hook::get_pattern("41 83 E0 1F 8B 44 81 08 44 0F A3 C0", -31);
});

static HookFunction initFunction([]()
{
	uint64_t* _id_CPedHeadBlendData = hook::get_address<uint64_t*>(hook::get_pattern("48 39 5E 38 74 1B 8B 15 ? ? ? ? 48 8D 4F 10 E8", 8));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_EYE_COLOR", [=](fx::ScriptContext& context)
	{
		int result = -1;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		if (entity && entity->IsOfType<CPed>())
		{
			auto address = (char*)entity + 16;
			auto table = g_extensionList_get(address, *_id_CPedHeadBlendData);
			if (table)
			{
				result = *(uint8_t*)(table + 310);
			}
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_FACE_FEATURE", [=](fx::ScriptContext& context)
	{
		float result = 0.0f;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		int index = context.GetArgument<int>(1);

		if (index < 20 && entity && entity->IsOfType<CPed>())
		{
			auto address = (char*)entity;
			if (*(BYTE *)(*(uint64_t *)(address + 32) + 646) & 2)
			{
				auto table = g_extensionList_get(address + 16, *_id_CPedHeadBlendData);
				if (table)
				{
					result = *(float *)(table + 4 * index + 160);
				}
			}
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_HEAD_OVERLAY_DATA", [=](fx::ScriptContext& context)
	{
		bool result = false;

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		int index = context.GetArgument<int>(1);

		if (entity && entity->IsOfType<CPed>())
		{
			auto address = (char*)entity;
			if (*(BYTE *)(*(uint64_t *)(address + 32) + 646) & 2)
			{
				auto table = g_extensionList_get(address + 16, *_id_CPedHeadBlendData);
				if (table)
				{
					*context.GetArgument<int*>(2) = *(uint8_t *)(table + 285 + index); // overlay value
					*context.GetArgument<int*>(3) = *(uint8_t *)(table + 266 + index); // colour type
					*context.GetArgument<int*>(4) = *(uint8_t *)(table + 240 + index); // main colour
					*context.GetArgument<int*>(5) = *(uint8_t *)(table + 253 + index); // secondary colour
					*context.GetArgument<float*>(6) = *(float *)(table + 56 + 4 * index); // overlay alpha

					result = true;
				}
			}
		}

		context.SetResult<bool>(result);
	});
});
