/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"
#include "Hooking.h"
#include "scrEngine.h"

class ComponentInstance : public Component
{
public:
	virtual bool Initialize();

	virtual bool Shutdown();

	virtual bool DoGameLoad(void* module);
};

bool ComponentInstance::Initialize()
{
	InitFunctionBase::RunAll();

	return true;
}

static void*(*oldInitCall)(void*, uint32_t, uint32_t);

static bool g_scriptInited;

bool IsScriptInited()
{
	return g_scriptInited;
}

namespace rage
{
	void OnScriptReInit();
}

static void* PostScriptInit(void* arg, uint32_t a2, uint32_t a3)
{
	void* retval = oldInitCall(arg, a2, a3);

	trace("script init?\n");

	g_scriptInited = true;

	static std::once_flag of;

	std::call_once(of, [] {
		rage::scrEngine::OnScriptInit();
	});

	rage::OnScriptReInit();

	return retval;
}

bool ComponentInstance::DoGameLoad(void* module)
{
	/*static hook::inject_call<void, bool> scriptInit(0x4201B0);

	scriptInit.inject([] (bool dontStartScripts)
	{
		scriptInit.call(dontStartScripts);

		rage::scrEngine::OnScriptInit();
	})*/

	// PREPARE FOR PATTERN HELL
	//auto match = hook::pattern("89 1D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 48  83 C4 20 5B E9 ? ? ? ? 83 F9 08").count(1).get(0);

	// 372 obfuscated (tier 1, mutation/jumpception) script core with a fair depth level, now we'll have to hook this internally...
	auto match = hook::pattern("BA AC 35 1D 3A 48 8B 0D ? ? ? ? 41 B8 01 00 00 00 E8").count(1).get(0);

	oldInitCall = (decltype(oldInitCall))hook::get_call(match.get<void>(/*23*/18));

	hook::call(match.get<void>(/*23*/18), PostScriptInit);

	HookFunction::RunAll();

	// ignore startup/network_startup
	//hook::put<uint8_t>(0x809A81, 0xEB);

	return true;
}

bool ComponentInstance::Shutdown()
{
	return true;
}

extern "C" __declspec(dllexport) Component* CreateComponent()
{
	return new ComponentInstance();
}
