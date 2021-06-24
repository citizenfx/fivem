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
#include <MinHook.h>

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

void(*g_origScriptInit)(bool);

void ScriptInitHook(bool dontStartScripts)
{
	g_origScriptInit(dontStartScripts);
	rage::scrEngine::OnScriptInit();
}

bool ComponentInstance::DoGameLoad(void* module)
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("C6 05 ? ? ? ? 00 C7 05 ? ? ? ? FF FF FF FF C6 05 ? ? ? ? 00 C7 05 ? ? ? ? 00 00 00 00", -0x5f), ScriptInitHook, (void**)&g_origScriptInit);
	MH_EnableHook(MH_ALL_HOOKS);

	HookFunction::RunAll();

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
