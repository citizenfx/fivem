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

static void(*oldInitCall)(void*);

static void PostScriptInit(void* arg)
{
	oldInitCall(arg);

	rage::scrEngine::OnScriptInit();
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
	auto match = hook::pattern("89 1D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 48  83 C4 20 5B E9 ? ? ? ? 83 F9 08").count(1).get(0);

	oldInitCall = (decltype(oldInitCall))hook::get_call(match.get<void>(23));

	hook::jump(match.get<void>(23), PostScriptInit);

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