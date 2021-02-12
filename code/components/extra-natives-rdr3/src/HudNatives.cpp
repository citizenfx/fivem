#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>
#include <MinHook.h>

static void* g_sm_bootstrapInstance;
static void* g_uiMinimap;

static hook::cdecl_stub<void(void*, char)> g_uiMinimap_SetType([]()
{
	return hook::get_pattern("48 81 EC ? ? ? ? 8B F2 48 8B E9 E8", -16);
});

static HookFunction hookFunction([]()
{
	{
		g_sm_bootstrapInstance = hook::get_address<void*>(hook::get_pattern("48 8B 06 48 8B CE FF 90 ? ? ? ? 44 8B F0", 22));

		uint32_t uiMinimapOffset = *hook::get_pattern<uint32_t>("33 D2 48 8B CB 45 8D 44 24 ? E8", -16);

		g_uiMinimap = (char*)g_sm_bootstrapInstance + uiMinimapOffset;
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_MINIMAP_TYPE", [](fx::ScriptContext& context)
	{
		auto minimapType = context.GetArgument<char>(0);

		if (g_uiMinimap)
			g_uiMinimap_SetType(g_uiMinimap, minimapType);
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
