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

static void* PostScriptInit(void* arg, uint32_t a2, uint32_t a3)
{
	void* retval = oldInitCall(arg, a2, a3);

	rage::scrEngine::OnScriptInit();

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
	auto match = hook::pattern("BA 2F 7B 2E 30 41 B8 0A").count(1).get(0);

	oldInitCall = (decltype(oldInitCall))hook::get_call(match.get<void>(/*23*/11));

	hook::call(match.get<void>(/*23*/11), PostScriptInit);

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