#include <StdInc.h>
#include <Hooking.h>
#include <MinHook.h>

int32_t* ms_entityLevelCap = NULL; //rage::fwMapData::ms_entityLevelCap
static void(*g_function)();
static void hk_function()
{
	// doing this because game keeps setting rage::fwMapData::ms_entityLevelCap to PRI_OPTIONAL_MEDIUM
	if (!g_function)
	{
		return g_function();
	}

	hook::put<int32_t>(ms_entityLevelCap, 0x03); 
}

static HookFunction hookFunction([]()
{
    	ms_entityLevelCap = hook::get_address<int32_t*>(hook::get_pattern<uint8_t>("0F 45 C2 89 05 ? ? ? ? 89 05", 0xB));

		MH_Initialize();
		MH_CreateHook((hook::get_call((hook::get_pattern("0F 47 C7 88 05", 0x9)))), hk_function, reinterpret_cast<void**>(&g_function));
		MH_EnableHook(MH_ALL_HOOKS);
})