/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

static void(*dataFileMgr__loadDat)(void*, const char*, bool);

static void LoadLevelDatHook(void* dataFileMgr, const char* name, bool enabled)
{
	dataFileMgr__loadDat(dataFileMgr, "citizen:/citizen.meta", enabled);
	dataFileMgr__loadDat(dataFileMgr, name, enabled);
}

static HookFunction hookFunction([] ()
{
	// level load
	void* hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B0 01 48 8B D3").count(1).get(0).get<void>(18);

	hook::set_call(&dataFileMgr__loadDat, hookPoint);
	hook::call(hookPoint, LoadLevelDatHook);
});