/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

static void(*dataFileMgr__loadDat)(void*, const char*, bool);

static std::vector<std::string> g_beforeLevelMetas;
static std::vector<std::string> g_afterLevelMetas;

static void LoadLevelDatHook(void* dataFileMgr, const char* name, bool enabled)
{
	dataFileMgr__loadDat(dataFileMgr, "citizen:/citizen.meta", enabled);

	// load before-level metas
	for (const auto& meta : g_beforeLevelMetas)
	{
		dataFileMgr__loadDat(dataFileMgr, meta.c_str(), enabled);
	}

	// load the level
	dataFileMgr__loadDat(dataFileMgr, name, enabled);

	// load after-level metas
	for (const auto& meta : g_afterLevelMetas)
	{
		dataFileMgr__loadDat(dataFileMgr, meta.c_str(), enabled);
	}
}

namespace streaming
{
	void DLL_EXPORT AddMetaToLoadList(bool before, const std::string& meta)
	{
		((before) ? g_beforeLevelMetas : g_afterLevelMetas).push_back(meta);
	}
}

static HookFunction hookFunction([] ()
{
	// level load
	void* hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B0 01 48 8B D3").count(1).get(0).get<void>(18);

	hook::set_call(&dataFileMgr__loadDat, hookPoint);
	hook::call(hookPoint, LoadLevelDatHook);
});