#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>
#include <MinHook.h>

#include "atArray.h"
#include "fxScripting.h"
#include "Hooking.Stubs.h"
#include "Resource.h"
#include "om/OMPtr.h"

static void* g_sm_bootstrapInstance;
static void* g_uiMinimap;

static bool g_UI_VISIBILE_WHEN_DEAD = false;

static hook::cdecl_stub<void(void*, char)> g_uiMinimap_SetType([]()
{
	return hook::get_pattern("48 81 EC ? ? ? ? 8B F2 48 8B E9 E8", -16);
});

static hook::cdecl_stub<uint32_t(void*)> g_uiMinimap_GetType([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 83 F8 ? 74 ? 48 8D 15"));
});

static bool (*g_origUICondContextStoreEval)(hook::FlexStruct* self, void* a2);
static bool UICondContextStoreEval(hook::FlexStruct* self, void* a2)
{
	int contextHash = self->At<int>(0x10);
	
	if(g_UI_VISIBILE_WHEN_DEAD && contextHash == 0xFC83EE25) // HUD_CTX_IN_RESPAWN
	{
		return false;
	}
	
	return g_origUICondContextStoreEval(self, a2);
}

static bool (*g_origCAIConditionIsDead)(void* self, void* context);
static bool CAIConditionIsDead(void* self, void* context)
{
	if(g_UI_VISIBILE_WHEN_DEAD)
	{
		return false;
	}
	return g_origCAIConditionIsDead(self, context);
}

static bool (*g_origIsDeathCameraRunning)();
static bool IsDeathCameraRunning()
{
	if(g_UI_VISIBILE_WHEN_DEAD)
	{
		return false;
	}
	return g_origIsDeathCameraRunning();
}

static HookFunction hookFunction([]()
{
	{
		g_sm_bootstrapInstance = hook::get_address<void*>(hook::get_pattern("48 8B 06 48 8B CE FF 90 ? ? ? ? 44 8B F0", 22));

		uint32_t uiMinimapOffset = *hook::get_pattern<uint32_t>("33 D2 48 8B CB 45 8D 44 24 ? E8", -16);

		g_uiMinimap = (char*)g_sm_bootstrapInstance + uiMinimapOffset;
	}

	{
		g_origUICondContextStoreEval = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 57 48 83 EC ? 8B 05 ? ? ? ? 4C 8B C2 48 8B F9 89 44 24 ? 49 8B C8 48 8D 54 24 ? E8"), UICondContextStoreEval);
		g_origCAIConditionIsDead = hook::trampoline(
			hook::get_pattern(
				"8B 41 ? 48 8B 8C C2 ? ? ? ? 48 85 C9 74 ? 80 79 ? ? 74 ? 33 C9 48 85 C9 74 ? 8B 81 ? ? ? ? 25 ? ? ? ? 2B 05 ? ? ? ? 48 69 C8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 94 01 ? ? ? ? 48 8B CA 48 83 E1 ? 48 F7 DA 48 1B C0 48 23 C1 8B 88 ? ? ? ? C1 E1"
			), CAIConditionIsDead
		);
		g_origIsDeathCameraRunning = hook::trampoline(hook::get_pattern("48 83 EC ? 32 C0 38 05 ? ? ? ? 88 44 24"), IsDeathCameraRunning);
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_MINIMAP_TYPE", [](fx::ScriptContext& context)
	{
		auto minimapType = context.GetArgument<char>(0);

		if (g_uiMinimap)
			g_uiMinimap_SetType(g_uiMinimap, minimapType);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_MINIMAP_TYPE", [](fx::ScriptContext& context)
	{
		uint32_t minimapType = g_uiMinimap_GetType(g_uiMinimap);
		context.SetResult<int>(minimapType);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_UI_VISIBLE_WHEN_DEAD", [](fx::ScriptContext& context) 
	{
		bool value = context.GetArgument<bool>(0);
		g_UI_VISIBILE_WHEN_DEAD = value;

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			resource->OnStop.Connect([]()
			{
				g_UI_VISIBILE_WHEN_DEAD = false;
			});
		}
	});

	/*
	{
		char* location = hook::get_pattern<char>("48 81 C1 ? ? ? ? 33 D2 E8 ? ? ? ? 83 FE 02", 3);

		uint32_t offsetPromptMapSimple   = *(uint32_t*)location;
		uint32_t offsetPromptMapExpanded = *(uint32_t*)(location + 17);
		uint32_t offsetPromptMapRegular  = *(uint32_t*)(location + 38);
		uint32_t offsetPromptMapOff      = *(uint32_t*)(location + 58);
	}
	*/
});
