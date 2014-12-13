/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceManager.h"
#include "scrEngine.h"

__declspec(dllexport) GtaThread* TheScriptManager;

class ScriptManager : public GtaThread
{
private:
	bool m_errorState;

public:
	ScriptManager();

	virtual void DoRun();

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount);

	virtual void Kill();
};

ScriptManager::ScriptManager()
	: m_errorState(false)
{

}

void ScriptManager::DoRun()
{
	/*if (g_netLibrary->IsDisconnected())
	{
	int ped;
	int player;

	player = NativeInvoke::Invoke<0x62E319C6, int>(); // GET_PLAYER_ID

	NativeInvoke::Invoke<0x511454A9, int>(player, &ped); // GET_PLAYER_CHAR

	NativeInvoke::Invoke<0x689D0F5F, int>(ped, 9000.f, 9000.f, 200.f); // SET_CHAR_COORDINATES
	}*/

	if (m_errorState)
	{
		return;
	}

	TheResources.Tick();

	/*if (g_errorOccurredThisFrame)
	{
		m_errorState = true;
	}*/
}

rage::eThreadState ScriptManager::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	trace("script reset\n");

	/*auto& loadedResources = TheDownloads.GetLoadedResources();

	// and start all of them!
	for (auto& resource : loadedResources)
	{
		if (resource->GetState() == ResourceStateStopped)
		{
			resource->Start();
		}
	}

	TheDownloads.ClearLoadedResources();

	ScriptEnvironment::SignalScriptReset.emit();*/

	ResourceManager::OnScriptReset();

	m_errorState = false;

	return GtaThread::Reset(scriptHash, pArgs, argCount);
}

void ScriptManager::Kill()
{
	trace("script kill\n");

	return GtaThread::Kill();
}

static InitFunction initFunction([] ()
{
	rage::scrEngine::OnScriptInit.Connect([] ()
	{
		// create the script manager
		static ScriptManager* scriptManager = new ScriptManager();

		TheScriptManager = scriptManager;

		rage::scrEngine::CreateThread(scriptManager);
	});
});