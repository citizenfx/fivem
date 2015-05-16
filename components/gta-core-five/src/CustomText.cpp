/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <mutex>

static const char*(*g_origGetText)(void* theText, uint32_t hash);

static std::mutex g_textMutex;
static std::unordered_map<uint32_t, std::string> g_textMap;

static const char* GetText(void* theText, uint32_t hash)
{
	{
		std::unique_lock<std::mutex> lock(g_textMutex);

		auto it = g_textMap.find(hash);

		if (it != g_textMap.end())
		{
			return it->second.c_str();
		}
	}

	return g_origGetText(theText, hash);
}

void AddCustomText(const char* key, const char* value)
{
	std::unique_lock<std::mutex> lock(g_textMutex);
	g_textMap[HashString(key)] = value;
}

static HookFunction hookFunction([] ()
{
	g_textMap[HashString("PM_PANE_LEAVE")] = "Disconnect";

	void* getTextPtr = hook::pattern("48 8B CB 8B D0 E8 ? ? ? ? 48 85 C0 0F 95 C0").count(1).get(0).get<void>(5);
	hook::set_call(&g_origGetText, getTextPtr);
	hook::call(getTextPtr, GetText);

	hook::call(hook::pattern("48 85 C0 75 34 8B 0D").count(1).get(0).get<void>(-5), GetText);
});