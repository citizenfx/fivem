#include "StdInc.h"
#include "Text.h"
#include <MinHook.h>

const wchar_t* CText__GetCustom(const char* key)
{
	if (!key)
	{
		return nullptr;
	}

	return TheText->GetCustom(key);
}

static const wchar_t* (__fastcall* g_origGetText)(void* self, void* dummy, const char* text);

const wchar_t* __fastcall CText_GetHook(void* self, void* dummy, const char* text)
{
	auto t = CText__GetCustom(text);

	if (t)
	{
		return t;
	}

	return g_origGetText(self, dummy, text);
}

static HookFunction hookFunction([] ()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("56 8B F1 85 C0 75 3F E8", -0x12), CText_GetHook, (void**)&g_origGetText);
	MH_EnableHook(MH_ALL_HOOKS);
});
