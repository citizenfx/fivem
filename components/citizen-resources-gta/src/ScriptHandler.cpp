/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ResourceManager.h>
#include <scrEngine.h>

#include <GameInit.h>

extern fwRefContainer<fx::ResourceManager> g_resourceManager;

class TestScriptThread : public GtaThread
{
	virtual void DoRun() override
	{
		static bool initedGame = false;

		if (!initedGame)
		{
			initedGame = Instance<ICoreGameInit>::Get()->HasVariable("networkInited");

			return;
		}

		static std::once_flag of;

		std::call_once(of, []()
		{
			OnKillNetworkDone.Connect([]()
			{
				initedGame = false;
			});
		});

		g_resourceManager->Tick();
	}
};

TestScriptThread thread;
extern GtaThread* g_resourceThread;

static InitFunction initFunction([] ()
{
	rage::scrEngine::OnScriptInit.Connect([] ()
	{
		rage::scrEngine::CreateThread(&thread);
		g_resourceThread = &thread;
	});
});